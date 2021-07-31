/*
 * VIRTIO-BLK
 */

#include <stdint.h>
#include <malloc.h>
#include <pthread.h>
#include <string.h>

#include "virtio.h"
#include "address_space.h"
#include "device.h"

/* Feature bits */
#define VIRTIO_BLK_F_BARRIER        0x1     /* Does host support barriers? */
#define VIRTIO_BLK_F_SIZE_MAX       0x2     /* Indicates maximum segment size */
#define VIRTIO_BLK_F_SEG_MAX        0x4     /* Indicates maximum # of segments */
#define VIRTIO_BLK_F_GEOMETRY       0x10    /* Legacy geometry available  */
#define VIRTIO_BLK_F_RO             0x20    /* Disk is read-only */
#define VIRTIO_BLK_F_BLK_SIZE       0x40    /* Block size of disk is available*/
#define VIRTIO_BLK_F_SCSI           0x80    /* Supports scsi command passthru */
#define VIRTIO_BLK_F_FLUSH          0x200   /* Flush command supported */
#define VIRTIO_BLK_F_TOPOLOGY       0x400   /* Topology information is available */
#define VIRTIO_BLK_F_CONFIG_WCE     0x800   /* Writeback mode available in config */
#define VIRTIO_BLK_F_MQ             0x1000  /* support more than one vq */
#define VIRTIO_BLK_F_DISCARD        0x2000  /* DISCARD is supported */
#define VIRTIO_BLK_F_WRITE_ZEROES   0x4000  /* WRITE ZEROES is supported */

#define VIRTIO_BLK_FEATURES \
    (VIRTIO_BLK_F_SEG_MAX | VIRTIO_BLK_F_GEOMETRY | \
     VIRTIO_BLK_F_BLK_SIZE | VIRTIO_BLK_F_FLUSH | \
     VIRTIO_BLK_F_TOPOLOGY | VIRTIO_BLK_F_CONFIG_WCE | \
     VIRTIO_BLK_F_DISCARD | VIRTIO_BLK_F_WRITE_ZEROES)

/* And this is the final byte of the write scatter-gather list. */
#define VIRTIO_BLK_S_OK     0
#define VIRTIO_BLK_S_IOERR  1
#define VIRTIO_BLK_S_UNSUPP 2

/* These two define direction. */
#define VIRTIO_BLK_T_IN         0
#define VIRTIO_BLK_T_OUT        1

/* This bit says it's a scsi command, not an actual read or write. */
#define VIRTIO_BLK_T_SCSI_CMD   2

/* Cache flush command */
#define VIRTIO_BLK_T_FLUSH      4

/* Get device ID command */
#define VIRTIO_BLK_T_GET_ID     8

/* Discard command */
#define VIRTIO_BLK_T_DISCARD    11

/* Write zeroes command */
#define VIRTIO_BLK_T_WRITE_ZEROES   13

/* Barrier before this op. */
#define VIRTIO_BLK_T_BARRIER    0x80000000

#define VIRTIO_BLK_QUEUE_SIZE 256
#define VIRTIO_BLK_BLOCK_SIZE 512


/*
 * This comes first in the read scatter-gather list.
 * For legacy virtio, if VIRTIO_F_ANY_LAYOUT is not negotiated,
 * this is the first element of the read scatter-gather list.
 */
typedef struct _virtio_blk_outhdr {
    /* VIRTIO_BLK_T* */
    uint32_t type;
    /* io priority. */
    uint32_t ioprio;
    /* Sector (ie. 512 byte offset) */
    uint64_t sector;
} virtio_blk_outhdr;

typedef struct _virtio_blk_t
{
    virtio_dev_t vdev;

    pthread_mutex_t _mutex;
    pthread_cond_t _cond;

    vq_request_t *_req;

    const char *filename;
} virtio_blk_t;

typedef struct _virtio_blk_config_t
{
    /* The capacity (in 512-byte sectors). */
    uint64_t capacity;
    /* The maximum segment size (if VIRTIO_BLK_F_SIZE_MAX) */
    uint32_t size_max;
    /* The maximum number of segments (if VIRTIO_BLK_F_SEG_MAX) */
    uint32_t seg_max;
    /* geometry of the device (if VIRTIO_BLK_F_GEOMETRY) */
    struct virtio_blk_geometry {
        uint16_t cylinders;
        uint8_t heads;
        uint8_t sectors;
    } geometry;

    /* block size of device (if VIRTIO_BLK_F_BLK_SIZE) */
    uint32_t blk_size;

    /* the next 4 entries are guarded by VIRTIO_BLK_F_TOPOLOGY  */
    /* exponent for physical block per logical block. */
    uint8_t physical_block_exp;
    /* alignment offset in logical blocks. */
    uint8_t alignment_offset;
    /* minimum I/O size without performance penalty in logical blocks. */
    uint16_t min_io_size;
    /* optimal sustained I/O size in logical blocks. */
    uint32_t opt_io_size;

    /* writeback mode (if VIRTIO_BLK_F_CONFIG_WCE) */
    uint8_t wce;
    uint8_t unused;

    /* number of vqs, only available when VIRTIO_BLK_F_MQ is set */
    uint16_t num_queues;

    /* the next 3 entries are guarded by VIRTIO_BLK_F_DISCARD */
    /*
     * The maximum discard sectors (in 512-byte sectors) for
     * one segment.
     */
    uint32_t max_discard_sectors;
    /*
     * The maximum number of discard segments in a
     * discard command.
     */
    uint32_t max_discard_seg;
    /* Discard commands must be aligned to this number of sectors. */
    uint32_t discard_sector_alignment;

    /* the next 3 entries are guarded by VIRTIO_BLK_F_WRITE_ZEROES */
    /*
     * The maximum number of write zeroes sectors (in 512-byte sectors) in
     * one segment.
     */
    uint32_t max_write_zeroes_sectors;
    /*
     * The maximum number of segments in a write zeroes
     * command.
     */
    uint32_t max_write_zeroes_seg;
    /*
     * Set if a VIRTIO_BLK_T_WRITE_ZEROES request may result in the
     * deallocation of one or more of the sectors.
     */
    uint8_t write_zeroes_may_unmap;

    uint8_t unused1[3];
} __attribute__((packed)) virtio_blk_config_t;


static void
virtio_blk_init_config(virtio_blk_t *blk)
{
    uint64_t size;
    virtio_blk_config_t config;

    if (sizeof(virtio_blk_config_t) > 256)
        panic("%s: virtio_blk_config_t is too large\n", __func__);

    memset(&config, 0, sizeof(config));

    if (get_file_size(blk->filename, &size) < 0)
        panic("%s: get file (%s) size error\n", __func__, blk->filename);

    config.capacity = size >> 9;
    config.size_max = 0;
    config.seg_max = VIRTIO_BLK_QUEUE_SIZE - 2;

    config.geometry.cylinders = 0;
    config.geometry.heads = 0;
    config.geometry.sectors = 0;

    config.blk_size = VIRTIO_BLK_BLOCK_SIZE;
    config.physical_block_exp = 0;
    config.alignment_offset = 0;

    config.min_io_size = 0;
    config.opt_io_size = 0;
    config.wce = 1;

    config.num_queues = 0;
    config.max_discard_sectors = 0x3FFFFF;
    config.max_discard_seg = 1;

    config.discard_sector_alignment = 1;
    config.max_write_zeroes_sectors = 0x3FFFFF;
    config.max_write_zeroes_seg = 0;
    config.write_zeroes_may_unmap = 0;

    memcpy(blk->vdev.config, (uint8_t *) &config, sizeof(config));
}

static uint32_t
virtio_blk_config_readb(virtio_dev_t *dev, uint32_t addr)
{
    printf("%s: need to be implemented!\n", __func__);
    return 0;
}

static void
virtio_blk_config_writeb(virtio_dev_t *dev, uint32_t addr, uint32_t data)
{
    printf("%s: need to be implemented!\n", __func__);
}

static uint64_t
virtio_blk_get_features()
{
    uint64_t features = VIRTIO_COMMON_FEATURES;

    if ((features & VIRTIO_F_VERSION_1) == 0) {
        features &= ~VIRTIO_F_ANY_LAYOUT;
        features |= VIRTIO_BLK_F_SCSI;
    }

    return (features | VIRTIO_BLK_FEATURES);
}

static void
_complete(virtio_blk_t *blk, vq_request_t *req)
{
    as_write_nommu(NULL, req->iov[req->num-1].base, 1, VIRTIO_BLK_S_OK, 0);

    vring_used_write(blk->vdev.vq, req);

    blk->vdev.isr |= 0x1;
    plic_signal(blk->vdev.irq_num);
}

static int
_handle_read(virtio_blk_t *blk, vq_request_t *req, uint64_t sector)
{
    int i;
    FILE *fp;
    uint8_t *data;

    if (req->num < 3) {
        panic("%s: bad request number %d\n", __func__, req->num);
    }

    fp = fopen(blk->filename, "rb");
    if (fp == NULL)
        panic("%s: cannot open file %s\n", __func__, blk->filename);

    if (fseek(fp, sector * 512, SEEK_SET) < 0)
        panic("%s: cannot seek sector(%u) of file %s\n",
              __func__, sector, blk->filename);

    for (i = 1; i < (req->num - 1); i++) {
        data = malloc(req->iov[i].len);

        if (fread(data, 1, req->iov[i].len, fp) != req->iov[i].len)
            panic("%s: cannot read file %s\n", __func__, blk->filename);

        as_write_blob(req->iov[i].base, req->iov[i].len, data);
        free(data);
    }

    fclose(fp);

    _complete(blk, req);

    return 0;
}

static int
_handle_write(virtio_blk_t *blk, vq_request_t *req, uint64_t sector)
{
    FILE *fp;
    uint8_t *data;

    if (req->num != 3)
        panic("%s: bad request number %d\n", __func__, req->num);

    data = malloc(req->iov[1].len);
    as_read_blob(req->iov[1].base, req->iov[1].len, data);

    fp = fopen(blk->filename, "r+b");
    if (fp == NULL)
        panic("%s: cannot open file %s\n", __func__, blk->filename);

    if (fseek(fp, sector * 512, SEEK_SET) < 0)
        panic("%s: cannot seek sector(%u) of file %s\n",
              __func__, sector, blk->filename);

    if (fwrite(data, 1, req->iov[1].len, fp) != req->iov[1].len)
        panic("%s: cannot write file %s\n", __func__, blk->filename);

    fclose(fp);
    free(data);

    _complete(blk, req);

    return 0;
}

static int
_handle_flush(virtio_blk_t *blk, vq_request_t *req)
{
    if (req->num != 2)
        panic("%s: bad request number %d\n", __func__, req->num);

    _complete(blk, req);
    return 0;
}

static int
_do_request(virtio_blk_t *blk, vq_request_t *req)
{
    virtio_blk_outhdr outhdr;

    /* Request head */
    if ((req->iov[0].len != sizeof(virtio_blk_outhdr)) ||
        !(req->iov[0].flags & IO_VEC_F_READ)) {
        panic("%s: bad out header\n", __func__);
    }

    as_read_blob(req->iov[0].base, sizeof(virtio_blk_outhdr), (uint8_t *)&outhdr);

    switch (outhdr.type & ~(VIRTIO_BLK_T_OUT | VIRTIO_BLK_T_BARRIER))
    {
    case VIRTIO_BLK_T_IN:
        if (outhdr.type & VIRTIO_BLK_T_OUT)
            return _handle_write(blk, req, outhdr.sector);
        else
            return _handle_read(blk, req, outhdr.sector);

    case VIRTIO_BLK_T_FLUSH:
        return _handle_flush(blk, req);

    default:
        panic("%s: bad out type (%u)\n", __func__, outhdr.type);
    }

    return 0;
}

static void *
_routine(void *arg)
{
    virtio_blk_t *blk = (virtio_blk_t *) arg;

    while (1) {
        vq_request_t *req = NULL;

        pthread_mutex_lock(&blk->_mutex);

        while (blk->_req == NULL)
            pthread_cond_wait(&blk->_cond, &blk->_mutex);

        req = blk->_req;
        blk->_req = NULL;

        pthread_mutex_unlock(&blk->_mutex);

        _do_request(blk, req);
    }

    return NULL;
}

static int
virtio_blk_handle_request(virtio_dev_t *vdev, vq_request_t *req)
{
    virtio_blk_t *blk = (virtio_blk_t *) vdev;

    pthread_mutex_lock(&blk->_mutex);

    if (blk->_req)
        panic("%s: _req already exists\n", __func__);

    blk->_req = req;

    pthread_mutex_unlock(&blk->_mutex);
    pthread_cond_signal(&blk->_cond);

    return 0;
}

virtio_dev_t *
virtio_blk_init(const char *filename, uint32_t irq_num)
{
    pthread_t tid;
    virtio_blk_t *blk;

    blk = calloc(1, sizeof(virtio_blk_t));

    blk->filename = filename;

    blk->vdev.id = VIRTIO_ID_BLOCK;
    blk->vdev.irq_num = irq_num;
    blk->vdev.get_features = virtio_blk_get_features;

    blk->vdev.config_readb = virtio_blk_config_readb;
    blk->vdev.config_writeb = virtio_blk_config_writeb;

    blk->vdev.handle_request = virtio_blk_handle_request;

    virtio_blk_init_config(blk);

    blk->vdev.vq = calloc(1, sizeof(vqueue_t));
    blk->vdev.num_queues = 1;

    pthread_mutex_init(&blk->_mutex, NULL);
    pthread_cond_init(&blk->_cond, NULL);

    pthread_create(&tid, NULL, _routine, blk);

    return (virtio_dev_t *) blk;
}

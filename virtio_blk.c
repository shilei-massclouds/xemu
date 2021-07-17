/*
 * VIRTIO-BLK
 */

#include <stdint.h>
#include <malloc.h>

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

    const char *filename;
} virtio_blk_t;


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

static int
virtio_blk_handle_request(virtio_dev_t *vdev, vq_request_t *req)
{
    FILE *fp;
    uint8_t *data;
    virtio_blk_outhdr outhdr;
    virtio_blk_t *blk = (virtio_blk_t *) vdev;

    int i;
    for (i = 0; i < req->num; i++)
        printf("%s: desc addr(0x%lx) len(%lu) flags(0x%x)\n",
               __func__,
               req->iov[i].base, req->iov[i].len,
               req->iov[i].flags);

    if (req->num != 3)
        panic("%s: bad request number %d\n", __func__, req->num);

    /* Request head */
    if ((req->iov[0].len != sizeof(virtio_blk_outhdr)) ||
        !(req->iov[0].flags & IO_VEC_F_READ)) {
        panic("%s: bad out header\n", __func__);
    }

    read_blob(req->iov[0].base, sizeof(virtio_blk_outhdr), (uint8_t *)&outhdr);

    printf("%s: outhdr type(0x%x) ioprio(%u) sector(0x%lx)\n",
           __func__,
           outhdr.type, outhdr.ioprio, outhdr.sector);

    data = malloc(req->iov[1].len);

    fp = fopen(blk->filename, "rb");
    if (fp == NULL)
        panic("%s: cannot open file %s\n", __func__, blk->filename);

    if (fread(data, 1, req->iov[1].len, fp) != req->iov[1].len)
        panic("%s: cannot read file %s\n", __func__, blk->filename);

    fclose(fp);

    write_blob(req->iov[1].base, req->iov[1].len, data);

    free(data);

    write_nommu(NULL, req->iov[2].base, 1, VIRTIO_BLK_S_OK, 0);

    vring_used_write(vdev->vq, req);
    return 0;
}

virtio_dev_t *
virtio_blk_init(const char *filename, uint32_t irq_num)
{
    virtio_blk_t *blk;

    blk = calloc(1, sizeof(virtio_blk_t));

    blk->filename = filename;

    blk->vdev.id = VIRTIO_ID_BLOCK;
    blk->vdev.irq_num = irq_num;
    blk->vdev.get_features = virtio_blk_get_features;

    blk->vdev.config_readb = virtio_blk_config_readb;
    blk->vdev.config_writeb = virtio_blk_config_writeb;

    blk->vdev.handle_request = virtio_blk_handle_request;

    blk->vdev.config[0x01] = 0x08;
    blk->vdev.config[0x0c] = 0xFE;
    blk->vdev.config[0x15] = 0x02;
    blk->vdev.config[0x20] = 0x01;
    blk->vdev.config[0x24] = 0xFF;
    blk->vdev.config[0x25] = 0xFF;
    blk->vdev.config[0x26] = 0x3F;
    blk->vdev.config[0x28] = 0x01;
    blk->vdev.config[0x2c] = 0x01;
    blk->vdev.config[0x30] = 0xFF;
    blk->vdev.config[0x31] = 0xFF;
    blk->vdev.config[0x32] = 0x3F;

    blk->vdev.vq = calloc(1, sizeof(vqueue_t));
    blk->vdev.num_queues = 1;

    return (virtio_dev_t *) blk;
}

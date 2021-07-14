/*
 * VIRTIO-BLK
 */

#include <stdint.h>
#include <malloc.h>

#include "virtio.h"

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


typedef struct _virtio_blk_t
{
    virtio_dev_t vdev;
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
virtio_blk_get_features(uint64_t features)
{
    if ((features & VIRTIO_F_VERSION_1) == 0) {
        features &= ~VIRTIO_F_ANY_LAYOUT;
        features |= VIRTIO_BLK_F_SCSI;
    }

    return (features | VIRTIO_BLK_FEATURES);
}

virtio_dev_t *
virtio_blk_init()
{
    virtio_blk_t *blk;

    blk = calloc(1, sizeof(virtio_blk_t));
    blk->vdev.id = VIRTIO_ID_BLOCK;
    blk->vdev.get_features = virtio_blk_get_features;

    blk->vdev.config_readb = virtio_blk_config_readb;
    blk->vdev.config_writeb = virtio_blk_config_writeb;

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

    return (virtio_dev_t *) blk;
}

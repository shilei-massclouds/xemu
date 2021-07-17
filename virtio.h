/*
 * VIRTIO
 */

#ifndef _VIRTIO_H_
#define _VIRTIO_H_

#include "types.h"
#include "util.h"

/* This marks a buffer as continuing via the next field. */
#define VRING_DESC_F_NEXT       0x1
/* This marks a buffer as write-only (otherwise read-only). */
#define VRING_DESC_F_WRITE      0x2
/* This means the buffer contains a list of buffer descriptors. */
#define VRING_DESC_F_INDIRECT   0x4

/*
 * MMIO control registers
 */

/* Magic value ("virt" string) - Read Only */
#define VIRTIO_MMIO_MAGIC_VALUE         0x000

/* Virtio device version - Read Only */
#define VIRTIO_MMIO_VERSION             0x004

/* Virtio device ID - Read Only */
#define VIRTIO_MMIO_DEVICE_ID           0x008

/* Virtio vendor ID - Read Only */
#define VIRTIO_MMIO_VENDOR_ID           0x00c

/* Bitmask of the features supported by the device (host)
 * (32 bits per set) - Read Only */
#define VIRTIO_MMIO_DEVICE_FEATURES	    0x010

/* Device (host) features set selector - Write Only */
#define VIRTIO_MMIO_DEVICE_FEATURES_SEL	0x014

/* Bitmask of features activated by the driver (guest)
 * (32 bits per set) - Write Only */
#define VIRTIO_MMIO_DRIVER_FEATURES	0x020

/* Activated features set selector - Write Only */
#define VIRTIO_MMIO_DRIVER_FEATURES_SEL	0x024

/* Guest's memory page size in bytes - Write Only */
#define VIRTIO_MMIO_GUEST_PAGE_SIZE	0x028

/* Queue selector - Write Only */
#define VIRTIO_MMIO_QUEUE_SEL		0x030

/* Maximum size of the currently selected queue - Read Only */
#define VIRTIO_MMIO_QUEUE_NUM_MAX	0x034

/* Queue size for the currently selected queue - Write Only */
#define VIRTIO_MMIO_QUEUE_NUM		0x038

/* Used Ring alignment for the currently selected queue - Write Only */
#define VIRTIO_MMIO_QUEUE_ALIGN		0x03c

/* Guest's PFN for the currently selected queue - Read Write */
#define VIRTIO_MMIO_QUEUE_PFN		0x040

/* Ready bit for the currently selected queue - Read Write */
#define VIRTIO_MMIO_QUEUE_READY		0x044

/* Queue notifier - Write Only */
#define VIRTIO_MMIO_QUEUE_NOTIFY	0x050

/* Interrupt status - Read Only */
#define VIRTIO_MMIO_INTERRUPT_STATUS	0x060

/* Interrupt acknowledge - Write Only */
#define VIRTIO_MMIO_INTERRUPT_ACK	0x064

/* Device status register - Read Write */
#define VIRTIO_MMIO_STATUS		0x070

/* Selected queue's Descriptor Table address, 64 bits in two halves */
#define VIRTIO_MMIO_QUEUE_DESC_LOW	0x080
#define VIRTIO_MMIO_QUEUE_DESC_HIGH	0x084

/* Selected queue's Available Ring address, 64 bits in two halves */
#define VIRTIO_MMIO_QUEUE_AVAIL_LOW	0x090
#define VIRTIO_MMIO_QUEUE_AVAIL_HIGH	0x094

/* Selected queue's Used Ring address, 64 bits in two halves */
#define VIRTIO_MMIO_QUEUE_USED_LOW	0x0a0
#define VIRTIO_MMIO_QUEUE_USED_HIGH	0x0a4

/* Configuration atomicity value */
#define VIRTIO_MMIO_CONFIG_GENERATION	0x0fc

/* The config space is defined by each driver as
 * the per-driver configuration space - Read Write */
#define VIRTIO_MMIO_CONFIG		0x100


/*
 * Reg values
 */

#define VIRT_MAGIC              0x74726976  /* 'virt' */
#define VIRT_VENDOR             0x554D4558  /* 'XEMU' */

#define VIRT_VERSION_LEGACY         1
#define VIRT_VERSION                2


/*
 * VIRTIO type
 */

#define VIRTIO_ID_NONE              0  /* virtio none */

#define VIRTIO_ID_NET               1  /* virtio net */
#define VIRTIO_ID_BLOCK             2  /* virtio block */
#define VIRTIO_ID_CONSOLE           3  /* virtio console */
#define VIRTIO_ID_RNG               4  /* virtio rng */
#define VIRTIO_ID_BALLOON           5  /* virtio balloon */
#define VIRTIO_ID_IOMEM             6  /* virtio ioMemory */
#define VIRTIO_ID_RPMSG             7  /* virtio remote processor messaging */
#define VIRTIO_ID_SCSI              8  /* virtio scsi */
#define VIRTIO_ID_9P                9  /* 9p virtio console */
#define VIRTIO_ID_MAC80211_WLAN     10 /* virtio WLAN MAC */
#define VIRTIO_ID_RPROC_SERIAL      11 /* virtio remoteproc serial link */
#define VIRTIO_ID_CAIF              12 /* Virtio caif */
#define VIRTIO_ID_MEMORY_BALLOON    13 /* virtio memory balloon */
#define VIRTIO_ID_GPU               16 /* virtio GPU */
#define VIRTIO_ID_CLOCK             17 /* virtio clock/timer */
#define VIRTIO_ID_INPUT             18 /* virtio input */
#define VIRTIO_ID_VSOCK             19 /* virtio vsock transport */
#define VIRTIO_ID_CRYPTO            20 /* virtio crypto */
#define VIRTIO_ID_SIGNAL_DIST       21 /* virtio signal distribution device */
#define VIRTIO_ID_PSTORE            22 /* virtio pstore device */
#define VIRTIO_ID_IOMMU             23 /* virtio IOMMU */
#define VIRTIO_ID_MEM               24 /* virtio mem */
#define VIRTIO_ID_FS                26 /* virtio filesystem */
#define VIRTIO_ID_PMEM              27 /* virtio pmem */
#define VIRTIO_ID_MAC80211_HWSIM    29 /* virtio mac80211-hwsim */

/* Features */

/* Do we get callbacks when the ring is completely used, even if we've
 * suppressed them? */
#define VIRTIO_F_NOTIFY_ON_EMPTY    0x01000000

/* Can the device handle any descriptor layout? */
#define VIRTIO_F_ANY_LAYOUT         0x08000000

/* We support indirect buffer descriptors */
#define VIRTIO_RING_F_INDIRECT_DESC 0x10000000

/* The Guest publishes the used index for which it expects an interrupt
 * at the end of the avail ring. Host should ignore the avail->flags field. */
/* The Host publishes the avail index for which it expects a kick
 * at the end of the used ring. Guest should ignore the used->flags field. */
#define VIRTIO_RING_F_EVENT_IDX     0x20000000

/* v1.0 compliant. */
#define VIRTIO_F_VERSION_1          0x100000000

#define VIRTIO_COMMON_FEATURES \
    (VIRTIO_F_NOTIFY_ON_EMPTY | VIRTIO_F_ANY_LAYOUT | \
     VIRTIO_RING_F_INDIRECT_DESC | VIRTIO_RING_F_EVENT_IDX)

#define VIRTIO_QUEUE_MAX 1024

#define VIRTQUEUE_MAX_SIZE 1024

typedef struct _vring_desc_t
{
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
} vring_desc_t;

typedef struct _vring_avail_t
{
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[];
} vring_avail_t;

typedef struct _vring_used_elem_t
{
    uint32_t id;
    uint32_t len;
} vring_used_elem_t;

typedef struct _vring_used_t
{
    uint16_t flags;
    uint16_t idx;
    vring_used_elem_t ring[];
} vring_used_t;

typedef struct _vring_t
{
    unsigned int num;
    unsigned int num_default;
    unsigned int align;

    uint64_t desc;
    uint64_t avail;
    uint64_t used;
} vring_t;

typedef struct _vqueue_t
{
    vring_t vring;

    /* Next head to pop */
    uint16_t last_avail_idx;
    uint16_t used_idx;
} vqueue_t;

typedef struct _vq_request_t
{
    uint32_t index;
    uint32_t in_len;
    uint32_t num;
    iovec_t *iov;
} vq_request_t;

typedef struct _virtio_dev_t
{
    uint32_t    id;

    uint32_t    irq_num;

    bool        host_features_sel;
    uint64_t    guest_features;
    bool        guest_features_sel;
    uint32_t    guest_page_shift;
    uint16_t    queue_sel;
    uint8_t     status;

    vqueue_t    *vq;
    uint16_t    num_queues;

    uint8_t     config[256];

    uint32_t (*config_readb)(struct _virtio_dev_t *vdev, uint32_t addr);
    void (*config_writeb)(struct _virtio_dev_t *vdev,
                          uint32_t addr, uint32_t data);

    uint64_t (*get_features)();

    int (*handle_request)(struct _virtio_dev_t *vdev, vq_request_t *req);

} virtio_dev_t;


virtio_dev_t *
virtio_blk_init();

static inline uint64_t
vring_align(uint64_t addr, uint64_t align)
{
    return ALIGN_UP(addr, align);
}

vq_request_t *
vqueue_pop(vqueue_t *vq);

void
vring_init(vring_t *vring, uint64_t pfn, uint32_t page_shift);

void
vring_used_write(vqueue_t *vq, vq_request_t *req);

#endif /*  _VIRTIO_H_ */

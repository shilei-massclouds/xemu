/*
 * VIRTIO
 */

#ifndef _VIRTIO_H_
#define _VIRTIO_H_

#include <stdint.h>


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
#define VIRT_VENDOR             0x554D4551  /* 'QEMU' */

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


typedef struct _virtio_dev_t
{
    uint32_t id;

    uint32_t (*config_readb)(struct _virtio_dev_t *vdev, uint32_t addr);
    void (*config_writeb)(struct _virtio_dev_t *vdev,
                          uint32_t addr, uint32_t data);

} virtio_dev_t;


virtio_dev_t *
virtio_blk_init();

#endif /*  _VIRTIO_H_ */

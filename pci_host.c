/*
 * PCI_HOST
 */

#include <malloc.h>

#include "address_space.h"
#include "device.h"
#include "util.h"
#include "pci.h"

#define PCI_HOST_ADDRESS_SPACE_START 0x0000000030000000
#define PCI_HOST_ADDRESS_SPACE_END   0x000000003FFFFFFF

/* Red Hat */
#define PCI_VENDOR_ID_REDHAT             0x1b36
#define PCI_DEVICE_ID_REDHAT_PCIE_HOST   0x0008

/* Red Hat / Qumranet (for QEMU) -- see pci-ids.txt */
#define PCI_VENDOR_ID_REDHAT_QUMRANET    0x1af4
#define PCI_SUBVENDOR_ID_REDHAT_QUMRANET 0x1af4
#define PCI_SUBDEVICE_ID_QEMU            0x1100

#define PCI_CLASS_BRIDGE_HOST            0x0600


typedef struct _pci_host_t
{
    device_t dev;

    uint16_t command;
    uint16_t status;
    uint16_t class_id;
} pci_host_t;


static uint64_t
pci_host_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    pci_host_t *pci_host = (pci_host_t *) dev;
    uint64_t ret = 0;

    if (addr >= 256)
        return (uint64_t) (-1);

    switch (addr)
    {
    case PCI_VENDOR_ID:
        ret = PCI_VENDOR_ID_REDHAT;
        if (size == 4)
            ret |= (PCI_DEVICE_ID_REDHAT_PCIE_HOST << 16);
        break;
    case PCI_DEVICE_ID:
        ret = PCI_DEVICE_ID_REDHAT_PCIE_HOST;
        break;
    case PCI_COMMAND:
        ret = pci_host->command;
        break;
    case PCI_STATUS:
        ret = pci_host->status;
        break;
    case PCI_CLASS_REVISION:
        ret = 0;
        if (size == 4)
            ret |= (PCI_CLASS_BRIDGE_HOST << 16);
        break;
    case PCI_CLASS_DEVICE:
        ret = PCI_CLASS_BRIDGE_HOST;
        break;
    case PCI_HEADER_TYPE:
        ret = PCI_HEADER_TYPE_NORMAL;
        break;
    case PCI_BASE_ADDRESS_0:
    case PCI_BASE_ADDRESS_1:
    case PCI_BASE_ADDRESS_2:
    case PCI_BASE_ADDRESS_3:
    case PCI_BASE_ADDRESS_4:
    case PCI_BASE_ADDRESS_5:
        break;
    case PCI_SUBSYSTEM_VENDOR_ID:
        ret = PCI_SUBVENDOR_ID_REDHAT_QUMRANET;
        break;
    case PCI_SUBSYSTEM_ID:
        ret = PCI_SUBDEVICE_ID_QEMU;
        break;
    case PCI_ROM_ADDRESS:
        break;
    case PCI_INTERRUPT_PIN:
        ret = 0;    /* 0 means that pin doesn't connect to intr lines */
        break;
    default:
        printf("%s: need to be implemented! [0x%lx]: (%lu)\n",
               __func__, addr, size);
        panic("%s: need to be implemented! [0x%lx]: (%lu)\n",
              __func__, addr, size);
    }

    return ret;
}

static uint64_t
pci_host_write(void *dev, uint64_t addr, uint64_t data, size_t size,
               params_t params)
{
    pci_host_t *pci_host = (pci_host_t *) dev;

    switch (addr)
    {
    case PCI_COMMAND:
        pci_host->command = (uint16_t) data;
        break;
    case PCI_BASE_ADDRESS_0:
    case PCI_BASE_ADDRESS_1:
    case PCI_BASE_ADDRESS_2:
    case PCI_BASE_ADDRESS_3:
    case PCI_BASE_ADDRESS_4:
    case PCI_BASE_ADDRESS_5:
        break;
    case PCI_ROM_ADDRESS:
        break;
    default:
        panic("%s: need to be implemented! [0x%lx]: 0x%lx (%u)\n",
              __func__, addr, data, size);
    }

    return 0;
}

device_t *
pci_host_init(address_space *parent_as)
{
    pci_host_t *pci_host;

    pci_host = calloc(1, sizeof(pci_host_t));
    pci_host->dev.name = "pci_host";

    init_address_space(&(pci_host->dev.as),
                       PCI_HOST_ADDRESS_SPACE_START,
                       PCI_HOST_ADDRESS_SPACE_END);

    pci_host->dev.as.ops.read_op = pci_host_read;
    pci_host->dev.as.ops.write_op = pci_host_write;

    pci_host->dev.as.device = pci_host;

    register_address_space(parent_as, &(pci_host->dev.as));

    return (device_t *) pci_host;
}

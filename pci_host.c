/*
 * PCI_HOST
 */

#include <malloc.h>

#include "address_space.h"
#include "device.h"

#define PCI_HOST_ADDRESS_SPACE_START 0x0000000030000000
#define PCI_HOST_ADDRESS_SPACE_END   0x000000003FFFFFFF

typedef struct _pci_host_t
{
    device_t dev;
} pci_host_t;


static uint64_t
pci_host_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    fprintf(stderr, "%s: need to be implemented!\n", __func__);
    return 0;
}

static uint64_t
pci_host_write(void *dev, uint64_t addr, uint64_t data, size_t size,
           params_t params)
{
    fprintf(stderr, "%s: need to be implemented!\n", __func__);
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

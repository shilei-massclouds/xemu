/*
 * AddressSpace
 */

#ifndef ADDRESS_SPACE_H
#define ADDRESS_SPACE_H

#include <stdio.h>

#include "types.h"

#define _PFN(addr) ((addr) >> PAGE_SHIFT)
#define IN_SAME_PAGE(addr, size) (_PFN(addr) == _PFN(addr + size - 1UL))

/* Sv39 for riscv64 */
#define ROOT_ADDRESS_SPACE_START 0x0000000000000000
#define ROOT_ADDRESS_SPACE_END   0x0000007FFFFFFFFF

/* Address space write_op 32-bit params */
typedef enum _params_t {
    PARAMS_NONE = 0,
    PARAMS_LR_SC,
    PARAMS_AMO_MIN,
    PARAMS_AMO_MAX,
    PARAMS_AMO_MINU,
    PARAMS_AMO_MAXU,
    PARAMS_AMO_ADD,
    PARAMS_AMO_XOR,
    PARAMS_AMO_OR,
    PARAMS_AMO_AND,
    PARAMS_AMO_SWAP,

    PARAMS_LAST
} params_t;

typedef struct _address_space
{
    uint64_t start;
    uint64_t end;

    struct {
        uint64_t (*read_op)(void *dev, uint64_t addr, size_t size,
                            params_t params);

        uint64_t (*write_op)(void *dev, uint64_t addr,
                             uint64_t data, size_t size,
                             params_t params);
    } ops;

    void *device;

    struct _address_space *children;
    struct _address_space *sibling;

} address_space;

void
init_address_space(address_space *as, uint64_t start, uint64_t end);

void
register_address_space(address_space *parent, address_space *child);

uint64_t
as_read_nommu(address_space *as, uint64_t addr, size_t size,
           params_t params);

uint64_t
as_write_nommu(address_space *as, uint64_t addr,
            size_t size, uint64_t data, params_t params);

uint64_t
as_read(address_space *as, uint64_t vaddr, size_t size,
     params_t params, bool *has_except);

uint64_t
as_write(address_space *as, uint64_t vaddr, size_t size, uint64_t data,
      params_t params, bool *has_except);

void
as_read_blob(uint64_t addr, size_t size, uint8_t *data);

void
as_write_blob(uint64_t addr, size_t size, uint8_t *data);

#endif /* ADDRESS_SAPCE_H */

/*
 * PLIC
 */

#include <malloc.h>

#include "types.h"
#include "address_space.h"
#include "device.h"
#include "util.h"
#include "csr.h"

#define PLIC_ADDRESS_SPACE_START 0x000000000C000000
#define PLIC_ADDRESS_SPACE_END   0x000000000FFFFFFF

#define NUM_SOURCES 127

/* According to SiFive U74 Core */
typedef struct _plic_t
{
    device_t dev;

    uint32_t priority[NUM_SOURCES + 1];

    uint32_t mpt;       /* M-Mode priority threshold */
    uint32_t spt;       /* S-Mode priority threshold */

    uint32_t mcc;       /* M-Mode claim and complete */
    uint32_t scc;       /* S-Mode claim and complete */

    uint32_t mie[5];    /* M-Mode interrupt enable */
    uint32_t sie[5];    /* S-Mode interrupt enable */

    uint32_t pending[5];
} plic_t;

plic_t *plic;   /* Global plic */


static void
_bit_pos(uint32_t id, uint32_t *index, uint32_t *offset)
{
    if (id == 0)
        panic("%s: interrupt number cannot be 0\n", __func__);

    *index = id / 32;
    *offset = id % 32;
}

void
plic_signal(uint32_t id)
{
    uint32_t index;
    uint32_t offset;

    if (id == 0)
        panic("%s: interrupt number cannot be 0\n", __func__);

    _bit_pos(id, &index, &offset);
    plic->pending[index] |= (1 << offset);
}

static void
clear_pending_bit(uint32_t id)
{
    uint32_t index;
    uint32_t offset;

    if (id == 0)
        panic("%s: interrupt number cannot be 0\n", __func__);

    _bit_pos(id, &index, &offset);
    plic->pending[index] &= ~(1 << offset);
}

static uint64_t
plic_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    plic_t *plic = (plic_t *) dev;

    if ((addr % 4))
        panic("%s: addr 0x%lx not align to 4-bytes\n", __func__, addr);

    if (addr >= 0x2080 && addr <= 0x2090) {
        addr = (addr - 0x2080) / 4;
        return plic->sie[addr];
    }

    if (addr == 0x200004) {
        /* Claim */
        if (plic->mcc == 0)
            return 0;

        clear_pending_bit(plic->mcc);
        return plic->mcc;
    }

    if (addr == 0x201004) {
        /* Claim */
        if (plic->scc == 0)
            return 0;

        clear_pending_bit(plic->scc);
        return plic->scc;
    }

    panic("%s: need to be implemented! 0x%lx, %lu\n",
          __func__, addr, size);

    return 0;
}

static uint64_t
plic_write(void *dev, uint64_t addr, uint64_t data, size_t size,
           params_t params)
{
    plic_t *plic = (plic_t *) dev;

    if ((addr % 4))
        panic("%s: addr 0x%lx not align to 4-bytes\n", __func__, addr);

    if (addr <= 0x210) {
        addr /= 4;
        plic->priority[addr] = data & 0x7;
        return 0;
    }

    if (addr >= 0x2000 && addr <= 0x2010) {
        addr = (addr - 0x2000) / 4;
        plic->mie[addr] = (uint32_t) data;
        return 0;
    }

    if (addr >= 0x2080 && addr <= 0x2090) {
        addr = (addr - 0x2080) / 4;
        plic->sie[addr] = (uint32_t) data;
        return 0;
    }

    if (addr == 0x200000) {
        plic->mpt = (uint32_t) data;
        return 0;
    }

    if (addr == 0x201000) {
        plic->spt = (uint32_t) data;
        return 0;
    }

    if (addr == 0x200004) {
        /* Complete */
        if (plic->mcc != (uint32_t) data)
            panic("%s: bad complete (%u, %u)\n",
                  __func__, plic->mcc, (uint32_t) data);

        plic->mcc = 0;
        return 0;
    }

    if (addr == 0x201004) {
        /* Complete */
        if (plic->scc != (uint32_t) data)
            panic("%s: bad complete (%u, %u)\n",
                  __func__, plic->scc, (uint32_t) data);

        plic->scc = 0;
        return 0;
    }

    panic("%s: need to be implemented! [0x%lx]: %lx (%lu)\n",
          __func__, addr, data, size);

    return 0;
}

device_t *
plic_init(address_space *parent_as)
{
    plic = calloc(1, sizeof(plic_t));
    plic->dev.name = "plic";

    init_address_space(&(plic->dev.as),
                       PLIC_ADDRESS_SPACE_START,
                       PLIC_ADDRESS_SPACE_END);

    plic->dev.as.ops.read_op = plic_read;
    plic->dev.as.ops.write_op = plic_write;

    plic->dev.as.device = plic;

    register_address_space(parent_as, &(plic->dev.as));

    return (device_t *) plic;
}

static uint32_t
first_one(uint32_t bits)
{
    uint32_t ret = 0;

    while (ret < 32) {
        if ((bits & 1))
            return ret;

        ret++;
        bits = bits >> 1;
    }

    panic("%s: bad bits 0x%x\n", __func__, bits);
    return 0;
}

static void
_set_pending_bit(uint32_t dst, uint32_t src, uint32_t index)
{
    bool has_except = false;
    uint64_t data = csr_read(dst, &has_except);
    SET_BIT(data, index, BIT(csr_read(src, &has_except), index));
    csr_update(dst, data, CSR_OP_WRITE, &has_except);
}

uint32_t
check_interrupt()
{
    int i;
    uint32_t ret = 0;
    bool has_except = false;

    bool deleg = BIT(csr_read(MIDELEG, &has_except), 9);
    uint32_t *xie = deleg ? plic->sie : plic->mie;
    uint32_t xpt = deleg ? plic->spt : plic->mpt;
    uint32_t *xcc = deleg ? &plic->scc : &plic->mcc;

    for (i = 0; i < 5; i++) {
        uint32_t bits = plic->pending[i] & xie[i];
        if (bits) {
            uint32_t id = i * 32 + first_one(bits);
            if (plic->priority[id] > xpt) {
                *xcc = id;
                ret = id;
                break;
            }
        }
    }

    if (ret == 0)
        return 0;

    if (deleg) {
        if (BIT(csr_read(SSTATUS, &has_except), MS_SIE)) {
            if (priv == S_MODE)
                _set_pending_bit(SIP, SIE, 9);
            else if (priv == U_MODE)
                _set_pending_bit(SIP, SIE, 8);
        } else {
            return 0;
        }
    } else {
        if (BIT(csr_read(MSTATUS, &has_except), MS_MIE)) {
            if (priv == S_MODE)
                _set_pending_bit(MIP, MIE, 9);
            else if (priv == U_MODE)
                _set_pending_bit(MIP, MIE, 8);
            else if (priv == M_MODE)
                _set_pending_bit(MIP, MIE, 11);
        } else {
            return 0;
        }
    }

    return ret;
}

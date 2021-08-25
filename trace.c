/*
 * Trace
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "trace.h"
#include "regfile.h"
#include "csr.h"
#include "yaml.h"
#include "system_map.h"

static int trace_decode_en;
static int trace_execute_en = 0;
static uint64_t trace_pc_start = 0x80000000;
static uint64_t trace_pc_end = 0xffffffe00087ccac;

static uint64_t va_pa_offset = 0xffffffe000000000 - 0x80200000;

void
trace_decode(uint64_t   pc,
             op_t       op,
             uint32_t   rd,
             uint32_t   rs1,
             uint32_t   rs2,
             uint64_t   imm,
             uint32_t   csr_addr)
{
    if (!trace_decode_en || pc < trace_pc_start)
        return;

    fprintf(stderr, "[%lx]: %s; rd: %s; rs1: %s; rs2: %s; imm: %lx\n",
           pc, op_name(op),
           reg_name(rd), reg_name(rs1), reg_name(rs2), imm);

    if (op >= CSRRW && op <= CSRRCI)
        fprintf(stderr, "csr: %s\n\n", csr_name(csr_addr));
    else
        fprintf(stderr, "\n");
}

void
trace_execute(uint64_t   pc,
              op_t       op,
              uint32_t   rd,
              uint32_t   rs1,
              uint32_t   rs2,
              uint64_t   imm,
              uint32_t   csr_addr)
{
    if (!trace_execute_en || pc < trace_pc_start || pc > trace_pc_end)
        return;

    fprintf(stderr, "[%lx]: %s; rd: %s(0x%0lx); rs1: %s(0x%0lx); rs2: %s(0x%0lx); imm: 0x%0lx\n",
            pc,
            op_name(op),
            reg_name(rd), reg[rd],
            reg_name(rs1), reg[rs1],
            reg_name(rs2), reg[rs2],
            imm);

    if (op >= CSRRW && op <= CSRRCI)
        fprintf(stderr, "csr: %s\n\n", csr_name(csr_addr));
    else
        fprintf(stderr, "\n");
}

#define TRACE_ITEM_MAXNUM   64

typedef struct _trace_item {
    const char *name;
    uint64_t    addr;
    bool        is_pa;
    int         nargs;
} trace_item;

static int trace_num;
static trace_item trace_table[TRACE_ITEM_MAXNUM];

static trace_item *
_match_pc(uint64_t pc)
{
    int i;
    for (i = 0; i < trace_num; i++) {
        if (trace_table[i].addr == pc)
            return trace_table + i;
    }
    return NULL;
}

void
trace(uint64_t pc)
{
    int i;
    trace_item *item = _match_pc(pc);
    if (!item)
        return;

    printf("======================================\n");
    printf("%s: %lx\n\n", item->name, item->addr);
    for (i = 0; i < item->nargs; i++)
        printf("  a%d: 0x%-16lx\n", i, reg[REG_A0+i]);
    printf("======================================\n");
}

static int
trace_parse_cb(TOKEN token, const char *key, const char *value)
{
    trace_item *item;

    switch (token)
    {
    case TOKEN_OBJ:
        item = &trace_table[trace_num++];
        item->name = match_in_system_map(key, &item->addr);
        if (item->name == NULL)
            panic("%s: bad obj %s\n", __func__, item->name);
        break;
    case TOKEN_KV:
        item = &trace_table[trace_num - 1];
        if (strcmp(key, "is_pa") == 0 && strcmp(value, "true")) {
            item->is_pa = true;
            item->addr -= va_pa_offset;
        } else if (strcmp(key, "nargs") == 0) {
            item->nargs = atoi(value);
        }
        break;
    default:
        break;
    }

    return 0;
}

void
setup_trace_table(void)
{
    parse_yaml(trace_parse_cb);
}

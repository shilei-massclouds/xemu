/*
 * Trace
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "list.h"
#include "trace.h"
#include "regfile.h"
#include "csr.h"
#include "yaml.h"
#include "system_map.h"
#include "address_space.h"

typedef struct _exit_item {
    list_head   entry;
    uint64_t    addr;
} exit_item;

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
    bool        disabled;
    uint64_t    addr;
    bool        no_mmu;
    int         nargs;
    int         stack;
    bool        exit;
    list_head   exit_list;
} trace_item;

static int trace_num;
static trace_item trace_table[TRACE_ITEM_MAXNUM];

static uint64_t
_get_exit_addr(list_head *list)
{
    uint64_t addr;
    exit_item *p;

    if (list_empty(list))
        return 0;

    p = list_entry(list->prev, exit_item, entry);
    return p->addr;
}

static void
_pop_last_exit_addr(list_head *list)
{
    exit_item *p = list_entry(list->prev, exit_item, entry);
    list_del(list->prev);
    free(p);
}

static trace_item *
_match_pc(uint64_t pc, uint64_t *ret_pc)
{
    int i;
    for (i = 0; i < trace_num; i++) {
        trace_item *item = trace_table + i;
        uint64_t exit_pc = _get_exit_addr(&(item->exit_list));
        if (exit_pc && exit_pc == pc) {
            *ret_pc = exit_pc;
            _pop_last_exit_addr(&(item->exit_list));
            return trace_table + i;
        }

        if (trace_table[i].addr == pc) {
            return trace_table + i;
        }
    }
    return NULL;
}

static uint64_t
_read_dword(bool no_mmu, uint64_t base, int index)
{
    uint64_t addr = base + ((uint64_t)index << 3);

    if (no_mmu)
        return as_read_nommu(NULL, addr, 8, 0);

    return as_read(NULL, addr, 8, 0, NULL);
}

void
trace(uint64_t pc)
{
    int i;
    char point[32];
    uint64_t addr;
    uint64_t exit_pc = 0;
    trace_item *item = _match_pc(pc, &exit_pc);
    if (!item || item->disabled)
        return;

    if (exit_pc) {
        strcpy(point, "exit");
        addr = exit_pc;
    } else {
        strcpy(point, "entry");
        addr = item->addr;
    }

    printf("======================================\n");
    printf("%s(%s): 0x%lx\n\n", item->name, point, addr);

    for (i = 0; i < item->nargs; i++)
        printf("  a%d: 0x%-16lx\n", i, reg[REG_A0+i]);
    if (item->nargs)
        printf("\n");

    printf("  sp: 0x%lx\n", reg[REG_SP]);
    for (i = 0; i < item->stack; i++) {
        uint64_t value = _read_dword(item->no_mmu, reg[REG_SP], i);
        printf("  - 0x%-16lx\n", value);
    }
    printf("\n");

    if (exit_pc == 0)
        printf("  ra: 0x%lx\n", reg[REG_RA]);

    if (item->exit) {
        exit_item *p = malloc(sizeof(exit_item));
        p->addr = reg[REG_RA];
        list_add_tail(&(p->entry), &(item->exit_list));
    }

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
        INIT_LIST_HEAD(&(item->exit_list));
        break;
    case TOKEN_KV:
        item = &trace_table[trace_num - 1];
        if (streq(key, "disabled")) {
            item->disabled = streq(value, "true");
        } else if (streq(key, "no_mmu")) {
            item->no_mmu = streq(value, "true");
            item->addr -= va_pa_offset;
        } else if (streq(key, "nargs")) {
            item->nargs = atoi(value);
        } else if (streq(key, "stack")) {
            item->stack = atoi(value);
        } else if (streq(key, "exit")) {
            item->exit = streq(value, "true");
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

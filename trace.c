/*
 * Trace
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "isa.h"
#include "list.h"
#include "trace.h"
#include "regfile.h"
#include "csr.h"
#include "yaml.h"
#include "system_map.h"
#include "address_space.h"

typedef struct _watch_item {
    list_head   entry;
    const char  *name;
    char        type;
} watch_item;

static int trace_decode_en;
static int trace_execute_en = 0;
static uint64_t trace_pc_start = 0x80000000;
static uint64_t trace_pc_end = 0xffffffe00087ccac;

static uint64_t va_pa_offset = 0xffffffe000000000 - 0x80200000;

static inline uint64_t
va_to_pa(uint64_t va)
{
    return (va - va_pa_offset);
}

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
    bool        enabled;
    uint64_t    addr;
    bool        no_mmu;
    int         stack;
    int         ninst;
    list_head   watch_list;
} trace_item;

static int trace_num;
static trace_item trace_table[TRACE_ITEM_MAXNUM];
static trace_item *item_in_process;

static trace_item *
_match_pc(uint64_t pc)
{
    int i;

    if (item_in_process) {
        if (item_in_process->ninst > 0) {
            item_in_process->ninst--;
            return item_in_process;
        }
        item_in_process = NULL;
    }

    for (i = 0; i < trace_num; i++) {
        trace_item *item = trace_table + i;
        if (pc == item->addr) {
            if (item->ninst > 0)
                item_in_process = item;

            return item;
        }
    }

    return NULL;
}

static void
_show_string(bool no_mmu, uint64_t addr, const char *name)
{
    char c;
    int i = 0;
    char str[256] = {0};

    do {
        if (no_mmu)
            c = (char)as_read_nommu(NULL, addr, 1, 0);
        else
            c = (char)as_read(NULL, addr, 1, 0, NULL);

        if (i > 255)
            panic("%s: string too long %d\n", __func__, i);

        str[i++] = c;
        addr++;
    } while (c);

    printf("  %s: %s\n", name, str);
}

static uint64_t
_read_number(bool no_mmu, uint64_t addr, size_t size)
{
    if (no_mmu)
        return as_read_nommu(NULL, addr, size, 0);

    return as_read(NULL, addr, size, 0, NULL);
}

static size_t
_conv_to_size(char c)
{
    switch (c)
    {
    case 'd':
        return 8;
    case 'w':
        return 4;
    case 'h':
        return 2;
    case 'b':
        return 1;
    default:
        panic("%s: bad indicator %c\n", __func__, c);
    }

    return 0;
}

#define HAS_RD  0x1
#define HAS_RS1 0x2
#define HAS_RS2 0x4

void
trace(uint64_t pc, op_t op,
      uint32_t rd, uint32_t rs1, uint32_t rs2,
      uint64_t imm, uint32_t opcode)
{
    uint64_t i;
    watch_item *watch;
    uint32_t flag = 0;
    trace_item *item = _match_pc(pc);
    if (!item || !item->enabled)
        return;

    printf("======================================\n");
    printf("0x%lx:", pc);

    switch (opcode)
    {
    case OP_LOAD:
    case OP_LOAD_FP:
        printf("  %s %s, %ld(%s)\n",
               op_name(op), reg_name(rd), imm, reg_name(rs1));
        flag = HAS_RD | HAS_RS1;
        break;
    case OP_IMM:
    case OP_IMM_W:
        printf("  %s %s, %s, %ld\n",
               op_name(op), reg_name(rd), reg_name(rs1), imm);
        flag = HAS_RD | HAS_RS1;
        break;
    case OP_STORE:
    case OP_STORE_FP:
        printf("  %s %s, %ld(%s)\n",
               op_name(op), reg_name(rs2), imm, reg_name(rs1));
        flag = HAS_RS1 | HAS_RS2;
        break;
    case OP_BRANCH:
        printf("  %s %s, %s, 0x%lx\n",
               op_name(op), reg_name(rs1), reg_name(rs2), (imm + pc));
        flag = HAS_RS1 | HAS_RS2;
        break;
    case OP_JAL:
        printf("  %s %s, 0x%lx\n",
               op_name(op), reg_name(rd), (imm + pc));
        flag = HAS_RD;
        break;
    default:
        panic("%s: bad opcode %x for %s\n",
              __func__, opcode, op_name(op));
    }
    printf("\n");

    if (rd && (flag & HAS_RD))
        printf("  %s: 0x%-16lx\n", reg_name(rd), reg[rd]);
    if (rs1 && (flag & HAS_RS1) && (rs1 != rd))
        printf("  %s: 0x%-16lx\n", reg_name(rs1), reg[rs1]);
    if (rs2 && (flag & HAS_RS2) && (rs2 != rd))
        printf("  %s: 0x%-16lx\n", reg_name(rs2), reg[rs2]);

    printf("\n");

    printf("  sp: 0x%lx\n", reg[REG_SP]);
    for (i = 0; i < item->stack; i++) {
        uint64_t value = _read_number(item->no_mmu, reg[REG_SP] + (i << 3), 8);
        printf("  - 0x%-16lx\n", value);
    }
    printf("\n");

    printf("  tp: 0x%lx\n\n", reg[REG_TP]);

    list_for_each_entry(watch, &(item->watch_list), entry) {
        uint64_t base;

        if (match_in_system_map(watch->name, &base) == NULL)
            panic("%s: base name %s\n", __func__, watch->name);

        if (item->no_mmu)
            base = va_to_pa(base);

        if (watch->type == 's') {
            _show_string(item->no_mmu, base, watch->name);
        } else {
            uint64_t value = _read_number(item->no_mmu,
                                          base,
                                          _conv_to_size(watch->type));

            printf("  %s: 0x%-16lx\n", watch->name, value);
        }
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
        INIT_LIST_HEAD(&(item->watch_list));
        break;
    case TOKEN_KV:
        item = &trace_table[trace_num - 1];
        if (streq(key, "enabled")) {
            item->enabled = streq(value, "true");
        } else if (streq(key, "no_mmu")) {
            item->no_mmu = streq(value, "true");
            item->addr = va_to_pa(item->addr);
        } else if (streq(key, "stack")) {
            item->stack = atoi(value);
        } else if (streq(key, "ninst")) {
            item->ninst = atoi(value);
        } else if (streq(key, "watch")) {
            watch_item *p = malloc(sizeof(watch_item));
            const char *end = strchr(value, ',');
            if (end) {
                p->name = strndup(value, (size_t)(end - value));
                p->type = *(end + 1);
            } else {
                p->name = strdup(value);
                p->type = 'd';
            }
            list_add_tail(&(p->entry), &(item->watch_list));
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

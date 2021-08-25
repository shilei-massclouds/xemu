/*
 * System map file operation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "system_map.h"

#define SYSTEM_MAP_FILE     "./image/System.map"

typedef struct _system_map_item {
    const char *name;
    uint64_t    addr;
} system_map_item;

static system_map_item *table;
static size_t last;
static size_t size;

void
setup_system_map(void)
{
    FILE *fp;
    char line[256];

    fp = fopen(SYSTEM_MAP_FILE, "r");
    if (fp == NULL) {
        panic("%s: bad system map file %s\n",
              __func__, SYSTEM_MAP_FILE);
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        uint64_t addr;
        char *name;
        char *end = strchr(line, ' ');
        if (end == NULL)
            continue;

        addr = strtoul(line, &end, 16);

        name = end + 3;
        end = strchr(line, '\n');
        name = strndup(name, (size_t)(end - name));

        if (last >= size) {
            system_map_item *tbl;
            size += 64;
            tbl = realloc(table,
                          size * sizeof(system_map_item));
            if (tbl == NULL) {
                if (table)
                    free(table);
                panic("%s: realloc failed\n", __func__);
            }
            else {
                table = tbl;
            }
        }

        table[last].name = name;
        table[last].addr = addr;
        last++;
    }

    fclose(fp);
}

const char *
match_in_system_map(const char *name, uint64_t *paddr)
{
    int i;

    for (i = 0; i < last; i++) {
        if (strcmp(name, table[i].name) == 0) {
            *paddr = table[i].addr;
            return table[i].name;
        }
    }

    return NULL;
}

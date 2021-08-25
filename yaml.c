/*
 * YAML parse
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "yaml.h"

#define TRACE_FILE_IN_YAML  "./conf/trace.yml"

static void
parse_line(const char *line, parse_cb cb)
{
    const char *end;
    char name[256] = {0};
    char value[256] = {0};

    if (line == NULL || line[0] == '#')
        return;

    while (line[0] == ' ')
        line++;

    if (line[0] == 0)
        return;

    end = strchr(line, ':');
    if (end == NULL)
        return;

    strncpy(name, line, (size_t)(end - line));
    //printf("%s: object %s\n", __func__, name);

    if (*(end + 1) == '\n') {
        cb(TOKEN_OBJ, name, "");
        return;
    }

    strncpy(value, end + 2, sizeof(value));
    //printf("%s: value %s\n", __func__, value);
    cb(TOKEN_KV, name, value);
}

void
parse_yaml(parse_cb cb)
{
    FILE *fp;
    char line[256];

    fp = fopen(TRACE_FILE_IN_YAML, "r");
    if (fp == NULL) {
        panic("%s: bad trace config file %s\n",
              __func__, TRACE_FILE_IN_YAML);
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL)
        parse_line(line, cb);

    fclose(fp);
}

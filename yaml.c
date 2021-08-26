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
    const char *begin;
    const char *end;
    char name[256] = {0};
    char value[256] = {0};

    if (line == NULL)
        return;

    begin = line;
    while (*begin == ' ')
        begin++;

    if (*begin == 0 || *begin == '#')
        return;

    end = strchr(begin, ':');
    if (end == NULL)
        return;

    strncpy(name, begin, (size_t)(end - begin));

    begin = end + 1;
    if (*begin == '\n') {
        cb(TOKEN_OBJ, name, "");
        return;
    }

    if (*begin != ' ')
        panic("%s: bad line %s in yaml\n", __func__, line);

    /* Skip space that followed ':' */
    begin++;
    end = strchr(begin, '\n');
    strncpy(value, begin, (size_t)(end - begin));
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

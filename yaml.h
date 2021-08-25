/*
 * YAML
 */

#ifndef _YAML_H_
#define _YAML_H_

#include <stdint.h>

typedef enum _TOKEN {
    TOKEN_NONE = 0,
    TOKEN_OBJ,
    TOKEN_KV,
} TOKEN;

typedef int (*parse_cb)(TOKEN token, const char *key, const char *value);

void
parse_yaml(parse_cb cb);

#endif /* _YAML_H_ */

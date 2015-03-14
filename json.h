#ifndef _JSON_H_
#define _JSON_H_

#include "varstr.h"

#define JSON_SUCCEED 1
#define JSON_FAILURE 0

#define VALUE_SIZE_MAX 512

typedef enum {
    NUMBER,
    BOOLEAN,
    FLOAT,
    DOUBLE,
    STRING,
    ARRAY,
    OBJECT
}JSON_TYPE;

typedef struct json_value {
    JSON_TYPE type;
    int anonymous;
    char *name;
    union {
        char *string;
        long long number;
        int boolean;
        float float_decimal;
        double double_decimal;
        struct json_value *children;
    }value;
    struct json_value *next;
}json_value;

typedef struct json_root {
    struct json_value *elems;
}json_root;

char *escape_string(char *str, int str_len);
char *unescape_string(char *str, int str_len);

struct json_value *create_json_string(char *name, char *value);
struct json_value *create_json_boolean(char *name, int value);
struct json_value *create_json_number(char *name, long long value);
struct json_value *create_json_float(char *name, float value);
struct json_value *create_json_double(char *name, double value);
struct json_value *create_json_object(char *name);
struct json_value *create_json_array(char *name);
void release_json_value(struct json_value *value);

int json_value_insert_child(struct json_value *parent, struct json_value *child);

struct json_root *create_json_root();
int json_root_insert_value(struct json_root *root, struct json_value *value);
int release_json_root(json_root *root);

int json_serialize(struct json_root *root, struct varstr *str);
int json_deserialize(struct json_root *root, struct varstr *str);

struct json_value *json_find_value(struct json_root *root, char *name);

#endif

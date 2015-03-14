#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "varstr.h"

struct varstr *create_varstr()
{
    struct varstr *str = (struct varstr *)malloc(sizeof(*str));
    if(str == NULL) {
        return NULL;
    }

    str->data = NULL;
    str->cap = 0;
    str->len = 0;

    return str;
}

int expand_varstr(struct varstr *str, int len)
{
    int expand_size = 2 * len;
    char *new_space = (char *)malloc(str->cap + expand_size);
    if(new_space == NULL) {
        return 0;
    }
    memset(new_space, 0, str->cap + expand_size);
    strncpy(new_space, str->data, str->len);
    free(str->data);

    str->data = new_space;
    str->cap += expand_size;

    return 1;
}

int append_varstr(struct varstr *str, char *data, int len)
{
    if(str == NULL || data == NULL) {
        return 0;
    }

    if(str->cap - str->len <= len) {
        int res = expand_varstr(str, len);
        if(res == 0) {
            return 0;
        }
    }

    strncpy(str->data + str->len, data, len);
    str->len += len;

    return 1;
}

struct varstr *dup_varstr(struct varstr *src)
{
    if(src == NULL) {
        return NULL;
    }

    struct varstr *dst = create_varstr();
    if(dst == NULL) {
        return NULL;
    }

    dst->data = (char *)malloc(src->cap);
    if(dst->data == NULL) {
        free(dst);
        return NULL;
    }

    strncpy(dst->data, src->data, src->len);
    dst->cap = src->cap;
    dst->len = src->len;

    return dst;
}

int release_varstr(struct varstr *str)
{
    if(str != NULL) {
        if(str->data != NULL) {
            free(str->data);
            str->data = NULL;
        }

        free(str);
        return 1;
    }

    return 0;
}

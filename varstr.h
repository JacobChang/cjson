#ifndef _VARSTR_H_
#define _VARSTR_H_

typedef struct varstr {
    char *data;
    int cap;
    int len;
}varstr;

struct varstr *create_varstr();
int append_varstr(struct varstr *str, char *data, int len);
struct varstr *dup_varstr(struct varstr *src);
int release_varstr(struct varstr *str);

#endif

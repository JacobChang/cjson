#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json.h"

static const char *json_false = "false";
static const char *json_true  = "true";
static int jfalse = 0;
static int jtrue = 1;

#define JSON_MAX_SIZE 1024

char *escape_string(char *str, int str_len)
{
    if(str == NULL || str_len == 0) {
        return NULL;
    }

    char buffer[JSON_MAX_SIZE];
    memset(buffer, 0, JSON_MAX_SIZE);

    int i = 0, j = 0;
    while (i < str_len) {
        switch (str[i]) {
            case '\t':
            case '\f':
            case '\b':
            case '\n':
            case '\r':
            case '\"':
            case '\\':
                buffer[j++] = '\\';
            default:
                buffer[j++] = str[i++];
                break;
        }
    }

    char *dst = strndup(buffer, JSON_MAX_SIZE - 1);

    return dst;
}

char *unescape_string(char *str, int str_len)
{
    if(str == NULL || str_len == 0) {
        return NULL;
    }

    char buffer[JSON_MAX_SIZE];
    memset(buffer, 0, JSON_MAX_SIZE);

    int i = 0, j = 0;
    while(i < str_len) {
        if(str[i] == '\\') {
            i++;
        }

        buffer[j++] = str[i++];
    }

    char *dst = strndup(buffer, JSON_MAX_SIZE - 1);

    return dst;
}

struct json_value *create_json_value(JSON_TYPE type, char *name, int name_len, void *value, int value_len)
{
    struct json_value *elem = (struct json_value *)malloc(sizeof(*elem));
    if(elem == NULL) {
        return NULL;
    }
    elem->type = type;
    elem->next = NULL;
    elem->anonymous = 0;

    char *node_name = NULL, *node_value = NULL;
    node_name = escape_string(name, name_len);
    if(node_name == NULL) {
        free(elem);
        return NULL;
    }
    elem->name = node_name;

    switch(elem->type) {
        case NUMBER:
            elem->value.number = *(long long *)value;
            break;
        case STRING:
            node_value = escape_string(value, value_len);
            if(node_value == NULL && value_len != 0) {
                free(elem->name);
                free(elem);
                return NULL;
            }
            elem->value.string = node_value;
            break;
        case FLOAT:
            elem->value.float_decimal = *(float *)value;
            break;
        case DOUBLE:
            elem->value.double_decimal = *(double *)value;
            break;
        case ARRAY:
            elem->value.children = NULL;
            break;
        case OBJECT:
            elem->value.children = NULL;
            break;
        case BOOLEAN:
            elem->value.number = *(int *)value;
            break;
    }

    return elem;
}

struct json_value *init_json_value(JSON_TYPE type, char *name, void *value)
{
    struct json_value *value_node = (struct json_value *)malloc(sizeof(*value_node));
    if(value_node  == NULL) {
        return NULL;
    }

    value_node->type = type;
    value_node->name = name;
    value_node->anonymous = 0;
    value_node->next = NULL;
    switch(type) {
        case NUMBER:
            value_node->value.number = *(long long *)value;
        break;
        case STRING:
            value_node->value.string = (char *)value;
        break;
        case FLOAT:
            value_node->value.float_decimal = *(float *)value;
        break;
        case DOUBLE:
            value_node->value.double_decimal = *(double *)value;
        break;
        case ARRAY:
            value_node->value.children = NULL;
        break;
        case OBJECT:
            value_node->value.children = NULL;
        break;
        case BOOLEAN:
            value_node->value.boolean = *(int *)value;
        break;
    }

    return value_node;
}

struct json_value *create_json_string(char *name, char *value)
{
    int name_len, value_len;
    if(name == NULL) {
        name_len = 0;
    } else {
        name_len = strlen(name);
    }

    if(value == NULL) {
        value_len = 0;
    } else {
        value_len = strlen(value);
    }

    return create_json_value(STRING, name, name_len, value, value_len);
}

struct json_value *create_json_boolean(char *name, int value)
{
    if(name == NULL) {
        return NULL;
    }

    int name_len = strlen(name);
    if(name_len == 0) {
        return NULL;
    }

    return create_json_value(BOOLEAN, name, name_len, &value, 0);
}

struct json_value *create_json_number(char *name, long long value)
{
    if(name == NULL) {
        return NULL;
    }

    int name_len = strlen(name);
    if(name_len == 0) {
        return NULL;
    }

    return create_json_value(NUMBER, name, name_len, &value, 0);
}

struct json_value *create_json_float(char *name, float value)
{
    if(name == NULL) {
        return NULL;
    }

    int name_len = strlen(name);
    if(name_len == 0) {
        return NULL;
    }

    return create_json_value(FLOAT, name, name_len, &value, 0);
}

struct json_value *create_json_double(char *name, double value)
{
    if(name == NULL) {
        return NULL;
    }

    int name_len = strlen(name);
    if(name_len == 0) {
        return NULL;
    }

    return create_json_value(DOUBLE, name, name_len, &value, 0);
}

struct json_value *create_json_object(char *name)
{
    if(name == NULL) {
        return NULL;
    }

    int name_len = strlen(name);
    if(name_len == 0) {
        return NULL;
    }

    return create_json_value(OBJECT, name, name_len, NULL, 0);
}

struct json_value *create_json_array(char *name)
{
    if(name == NULL) {
        return NULL;
    }

    int name_len = strlen(name);
    if(name_len == 0) {
        return NULL;
    }

    return create_json_value(ARRAY, name, name_len, NULL, 0);
}

int json_value_insert_child(struct json_value *parent, struct json_value *child)
{
    if(parent == NULL || child == NULL) {
        return JSON_FAILURE;
    }

    if(parent->type == ARRAY || parent->type == OBJECT) {
        child->next = parent->value.children;
        parent->value.children = child;

        if(parent->type == ARRAY) {
            child->anonymous = 1;
        }

        return JSON_SUCCEED;
    }

    return JSON_FAILURE;
}

int json_value_serialize(struct json_value *elem, struct varstr *string)
{
    if(elem == NULL || string == NULL) {
        return JSON_FAILURE;
    }

    char buffer[JSON_MAX_SIZE] = {0};
    memset(buffer, 0, JSON_MAX_SIZE);

    int first = 1;

    struct json_value *child = NULL;

    if(elem->anonymous != 1) {
        append_varstr(string, "\"", 1);
        append_varstr(string, elem->name, strlen(elem->name));
        append_varstr(string, "\":", 2);
    }

    switch(elem->type) {
    case NUMBER:
        snprintf(buffer, JSON_MAX_SIZE, "%lld", elem->value.number);
        append_varstr(string, buffer, strlen(buffer));
        break;
    case DOUBLE:
        snprintf(buffer, JSON_MAX_SIZE, "%f", elem->value.double_decimal);
        append_varstr(string, buffer, strlen(buffer));
        break;
    case FLOAT:
        snprintf(buffer, JSON_MAX_SIZE, "%f", elem->value.float_decimal);
        append_varstr(string, buffer, strlen(buffer));
        break;
    case BOOLEAN:
        if(elem->value.boolean == 0) {
            append_varstr(string, json_false, strlen(json_false));
        } else {
            append_varstr(string, json_true, strlen(json_true));
        }
        break;
    case STRING:
        append_varstr(string, "\"", 1);
        if(elem->value.string != NULL) {
            append_varstr(string, elem->value.string, strlen(elem->value.string));
        }
        append_varstr(string, "\"", 1);
        break;
    case OBJECT:
        append_varstr(string, "{", 1);
        child = elem->value.children;
        while(child != NULL) {
            if( !first ) {
                append_varstr(string, ",", 1);
            }
            json_value_serialize(child, string);
            if( first ) {
                first = 0;
            }
            child = child->next;
        }
        append_varstr(string, "}", 1);
        break;
    case ARRAY:
        append_varstr(string, "[", 1);
        child = elem->value.children;
        while(child != NULL) {
            if( !first ) {
                append_varstr(string, ",", 1);
            }
            json_value_serialize(child, string);
            if( first ) {
                first = 0;
            }
            child = child->next;
        }
        append_varstr(string, "]", 1);
        break;
    }

    return JSON_SUCCEED;
}

int extract_string(char *data, int len, char **str)
{
    if(data == NULL || len == 0) {
        *str = NULL;
        return 0;
    }

    int i = 0, j = 0;

    while(data[i] == ' ' || data[i] == '\n' || data[i] == '\t' || data[i] == '\r') {
        i++;
    }

    if(data[i++] != '\"') {
        *str = NULL;
        return 0;
    }

    char buffer[JSON_MAX_SIZE];
    memset(buffer, 0, JSON_MAX_SIZE);

    int done = 0;
    int prev_is_esc = 0;
    
    while(i < len) {
        if(data[i] == '\\') {
	    if(!prev_is_esc) {
	        prev_is_esc = 1;
                buffer[j++] = data[i++];
                continue;
	    } else {
	        prev_is_esc = 0;
		buffer[j++] = data[i++];
		continue;
	    }
        }
        
        if(data[i] == '\n' || data[i] == '\r' || data[i] == '\t' || data[i] == '\b') {
	    if(!prev_is_esc) {
	        done = 0;
		break;
	    } else {
	        prev_is_esc = 0;
	        buffer[j++] = data[i++];
		continue;
	    }
	}

	if(data[i] == '\"') {
	    if(!prev_is_esc) {
	        i++;
	        done = 1;
		break;
	    } else {
	        prev_is_esc = 0;
	        buffer[j++] = data[i++];
		continue;
	    }
	}
	
	prev_is_esc = 0;

        buffer[j++] = data[i++];
    }
    
    if(done == 0) {
        *str = NULL;
        return 0;
    }

    char *res = strndup(buffer, JSON_MAX_SIZE);
    if(res != NULL) {
        *str = res;
    }

    return i;
}

int json_value_deserialize(struct json_value **value, char *rawdata, int maxlen, int anonymous)
{
    if(rawdata == NULL || maxlen == 0) {
        return JSON_FAILURE;
    }

    struct json_value *node = NULL, *child = NULL;
    char *node_name = NULL, *node_value = NULL;

    int len = 0, i = 0, j = 0;
    JSON_TYPE type = NUMBER;

    if(!anonymous) {
        len = extract_string(rawdata, maxlen, &node_name);
        if(len == 0) {
            return JSON_FAILURE;
        }
    }
    i += len;

    while(rawdata[i] == '\n' || rawdata[i] == ' ' || rawdata[i] == '\r' || rawdata[i] == '\t') {
        i++;
    }

    if(!anonymous) {
        if(rawdata[i++] != ':') {
            free(node_name);
            return JSON_FAILURE;
        }
    }

    while(rawdata[i] == '\n' || rawdata[i] == ' ' || rawdata[i] == '\r' || rawdata[i] == '\t') {
        i++;
    }

    switch(rawdata[i]) {
    case '[':
        i++;
        while(rawdata[i] == ' ' || rawdata[i] == '\n' || rawdata[i] == '\r' || rawdata[i] == '\t') {
            i++;
        }

        node = init_json_value(ARRAY, node_name, NULL);
        if(node == NULL) {
            free(node_name);
            return JSON_FAILURE;
        }

        if(rawdata[i] == ']') {
            *value = node;
            i++;
            break;
        }

        while(i < maxlen) {
            len = json_value_deserialize(&child, rawdata + i, maxlen - i, 1);
            if(len == 0) {
                release_json_value(node);
                return JSON_FAILURE;
            }
            i += len;
            json_value_insert_child(node, child);

            if(rawdata[i] == ',') {
                i++;
            }else if(rawdata[i] == ']') {
                break;
            }

            while(rawdata[i] == ' ' || rawdata[i] == '\n' || rawdata[i] == '\r' || rawdata[i] == '\t') {
                i++;
            }
        }
        *value = node;
        i++;
        break;
    case '{':
        i++;
        while(rawdata[i] == ' ' || rawdata[i] == '\n' || rawdata[i] == '\r' || rawdata[i] == '\t') {
            i++;
        }

        node = init_json_value(OBJECT, node_name, NULL);
        if(node == NULL) {
            free(node_name);
            return JSON_FAILURE;
        }

        if(rawdata[i] == '}') {
            *value = node;
            i++;
            break;
        }

        while(i < maxlen) {
            len = json_value_deserialize(&child, rawdata + i, maxlen - i, 0);
            if(len == 0) {
                release_json_value(node);
                return JSON_FAILURE;
            }
            i += len;
            json_value_insert_child(node, child);

            if(rawdata[i] == ',') {
                i++;
            }else if(rawdata[i] == '}') {
                break;
            }

            while(rawdata[i] == ' ' || rawdata[i] == '\n' || rawdata[i] == '\r' || rawdata[i] == '\t') {
                i++;
            }
        }
        *value = node;
        i++;
        break;
    case '\"':
        len = extract_string(rawdata + i, maxlen, &node_value);
        if(len == 0) {
            free(node_name);
            return JSON_FAILURE;
        }
        i += len;
        node = init_json_value(STRING, node_name, node_value);
        if(node == NULL) {
            free(node_name);
            free(node_value);

            return JSON_FAILURE;
        }

        while(rawdata[i] == ' ' || rawdata[i] == '\n' || rawdata[i] == '\r') {
            i++;
        }

        *value = node;
        break;
    default:
        j = i;
        if(rawdata[i] <= '9' && rawdata[i] >= '0') {
            while(rawdata[i] <= '9' && rawdata[i] >= '0') {
                i++;
            }

            if(rawdata[i] == '.') {
                type = FLOAT;
            }

            while(rawdata[i] <= '9' && rawdata[i] >= '0') {
                i++;
            }

            if(type == NUMBER) {
                long long number = atoll(rawdata + j);
                node = init_json_value(NUMBER, node_name, &number);
                if(node == NULL) {
                    free(node_name);
                    return JSON_FAILURE;
                }
            } else {
                float float_decimal = strtof(rawdata + j, NULL);
                node = init_json_value(FLOAT, node_name, &float_decimal);
                if(node == NULL) {
                    free(node_name);
                    return JSON_FAILURE;
                }
            }

            *value = node;
            break;
        } else {
            if(rawdata[i] == 't' && rawdata[i+1] == 'r' && rawdata[i+2] == 'u' && rawdata[i+3] == 'e') {
                node = init_json_value(BOOLEAN, node_name, &jtrue);
                i += 4;
            }

            if(rawdata[i] == 'f' && rawdata[i+1] == 'a' && rawdata[i+3] == 'l' && rawdata[i+3] == 's' && rawdata[i+4] == 'e') {
                node = init_json_value(BOOLEAN, node_name, &jfalse);
                i += 5;
            }
        }
    }

    return i;
}


void release_json_value(struct json_value *value)
{
    struct json_value *curr = NULL, *next = NULL;
    if(value != NULL) {
        if(value->name != NULL) {
            free(value->name);
            value->name = NULL;
        }
        switch(value->type) {
        case STRING:
            if(value->value.string != NULL) {
                free(value->value.string);
            }
            break;
        case NUMBER:
        case BOOLEAN:
        case FLOAT:
        case DOUBLE:
            break;
        case OBJECT:
        case ARRAY:
            curr = value->value.children;
            while(curr != NULL) {
                next = curr->next;
                release_json_value(curr);
                curr = next;
            }
            break;
        }
        free(value);
    }
}

struct json_root *create_json_root()
{
    struct json_root *root = (struct json_root *)malloc(sizeof(*root));
    if(root != NULL) {
        root->elems = NULL;
        return root;
    }

    return NULL;
}

int json_root_insert_value(struct json_root *root, struct json_value *value)
{
    if(root != NULL && value != NULL) {
        value->next = root->elems;
        root->elems = value;

        return JSON_SUCCEED;
    }

    return JSON_FAILURE;
}

int json_serialize(json_root *root, struct varstr *string)
{
    if(root == NULL || string == NULL) {
        return JSON_FAILURE;
    }

    struct json_value *elem = NULL;
    append_varstr(string, "{", 1);

    elem = root->elems;
    if(elem != NULL) {
        json_value_serialize(elem, string);
        elem = elem->next;
        while(elem != NULL) {
            append_varstr(string, ",", 1);
            int res = json_value_serialize(elem, string);
            if(res != JSON_SUCCEED) {
                return JSON_FAILURE;
            }
            elem = elem->next;
        }
    }

    append_varstr(string, "}", 1);

    return JSON_SUCCEED;
}

int json_deserialize(struct json_root *root, struct varstr *string)
{
    if(root == NULL || string == NULL || string->data == NULL) {
        return JSON_FAILURE;
    }

    char *rawdata = string->data;
    int len = string->len;

    if(rawdata[0] == '{' && rawdata[1] == '}') {
        return JSON_SUCCEED;
    }

    if(rawdata[0] != '{' || rawdata[len - 1] != '}') {
        return JSON_FAILURE;
    }

    struct json_value *value = NULL;
    int i = 1;
    int offset = 0;

    while(i < len - 1) {
        offset = json_value_deserialize(&value, rawdata + i, len - i, 0);
        json_root_insert_value(root, value);
        value = NULL;
        i += offset;

        while( rawdata[i] == ' ' || rawdata[i] == '\r' || rawdata[i] == '\n' || rawdata[i] == '\t') {
            i++;
        }

        if(rawdata[i] == ',') {
            i++;
        }
    }

    return JSON_SUCCEED;
}

int release_json_root(struct json_root *root)
{
    if(root != NULL) {
        struct json_value *curr = root->elems;
        struct json_value *next = NULL;
        while(curr != NULL) {
            next = curr->next;
            release_json_value(curr);
            curr = next;
        }

        root->elems = NULL;
        free(root);

        return JSON_SUCCEED;
    }

    return JSON_FAILURE;
}

struct json_value *json_find_value_same_level(struct json_value *value, char *name)
{
    while(value != NULL && name != NULL) {
        if(!strcasecmp(value->name, name)) {
            return value;
        }
        value = value->next;
    }

    return NULL;
}

struct json_value *json_find_value(struct json_root *root, char *name)
{
    if(root == NULL || name == NULL || root->elems == NULL) {
        return NULL;
    }

    struct json_value *value = NULL;
    struct json_value *target = NULL;
    char curr_name[64] = {0};

    int name_len = strlen(name);
    int i = 0, j;
    value = root->elems;
    while( i < name_len) {
        j = 0;
        while(*(name + i) != '>' && i < name_len) {
            curr_name[j++] = name[i++];
        }

        target = json_find_value_same_level(value, curr_name);
        if(target == NULL) {
            return NULL;
        } else {
            value = target->value.children;
        }
        memset(curr_name, 0, 64);
        i++;
    }

    return target;
}

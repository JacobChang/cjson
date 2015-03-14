#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "varstr.h"
#include "json.h"

void varstr_test()
{
    struct varstr *str = create_varstr();
    assert(str != NULL);

    char *sliceA = "sliceA";
    int sliceA_len = strlen(sliceA);
    int res = append_varstr(str, sliceA, sliceA_len);
    assert(res == 1);
    assert(str->len == sliceA_len);
    res = strcmp(sliceA, str->data);
    assert(res == 0);

    char *sliceB = "sliceBaa";
    int sliceB_len = strlen(sliceB);
    res = append_varstr(str, sliceB, sliceB_len);
    assert(res == 1);
    assert(str->len == sliceA_len + sliceB_len);

    struct varstr *dst = dup_varstr(str);
    assert( !strcmp(str->data, dst->data));
    assert(str->cap == dst->cap);
    assert(str->len == dst->len);

    release_varstr(str);
    release_varstr(dst);
}

void json_test()
{
    json_value *boolean = create_json_boolean("boolean", 1);
    assert(boolean->type == BOOLEAN);
    assert(boolean->value.boolean == 1);

    json_value *number = create_json_number("number", 100);
    assert(number->type == NUMBER);
    assert(number->value.number == 100);

    json_value *float_decimal = create_json_float("float", 1.0);
    assert(float_decimal->type == FLOAT);
    assert(float_decimal->value.float_decimal == 1.0);

    json_value *double_decimal = create_json_double("double", 2.0);
    assert(double_decimal->type == DOUBLE);
    assert(double_decimal->value.double_decimal == 2.0);

    json_value *string = create_json_string("string", "\"\\\t\n\rstring");
    assert(string->type == STRING);
    assert(strcmp(string->value.string, "\\\"\\\\\\\t\\\n\\\rstring") == 0);

    json_value *object = create_json_object("object");
    assert(object->type == OBJECT);
    assert(object->value.children == NULL);

    json_value *array = create_json_array("array");
    assert(array->type == ARRAY);
    assert(object->value.children == NULL);

    json_value_insert_child(array, number);
    json_value_insert_child(array, float_decimal);
    json_value_insert_child(array, double_decimal);
    json_value_insert_child(object, boolean);
    json_value_insert_child(object, string);
    json_value_insert_child(array, object);

    struct json_value *root = create_json_object("node");
    json_value_insert_child(root, array);

    struct varstr *str = create_varstr();
    json_serialize(root, str);

    struct json_value *target = json_find_value(root, "array");
    assert(target != NULL);
    assert(strcmp(target->name, array->name) == 0);
    assert(target->value.children == array->value.children);

    target = json_find_value(array, "number");
    assert(target != NULL);
    assert(strcmp(target->name, number->name) == 0);
    assert(target->value.number == number->value.number);

    release_json_value(root);
    release_varstr(str);
    str = NULL;
    root = NULL;

    char *json_data = "{\"object\":\r\n{\"\\\"\\\\\\\t\\\rstring\\\"\\\\\\\r\\\t\":\"\\\\\\\r\\\tstring\\\\\\\r\\\t\\\b\\\\\",\r\n\"number\":100},\r\n\"array\":[2,1]\r\n}";
    struct varstr *src = create_varstr();
    append_varstr(src, json_data, strlen(json_data));
    root = create_json_object("node");
    json_deserialize(root, src);

    struct varstr *dst = create_varstr();
    json_serialize(root, dst);
    assert(dst->len + 8 == strlen(json_data));

    release_json_value(root);
    release_varstr(src);
    release_varstr(dst);
}

int main(int argc, char **argv)
{
    varstr_test();
    json_test();

    return 0;
}

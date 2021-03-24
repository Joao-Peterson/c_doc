#include <stdio.h>
#include <stdlib.h>
#include "doc/doc.h"

// #define LOG_DISABLE

#ifndef LOG_DISABLE
    #define log(const_format_str, ...)  printf("[%s:%i] " const_format_str, __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define log(const_format_str, ...)  
#endif

#define STRUCT_NAME struct_ex_t
#define STRUCT_MEMBERS \
    X(int, value1)\
    X(int, value2)\
    X(double, value3)
#include "doc_struct.h"

int main(int argc, char **argv){

    // new dynamic object
    doc *obj = doc_new(
        "velocidades", dt_obj,
            "max", dt_double, 56.0,
            "media", dt_double, 22.5,
            "pontos", dt_array,
                "p1", dt_double, 23.0,        
                "p2", dt_double, 70.0,            
                "p3", dt_double, 104.3,        
                "p4", dt_double, 22.0,
            ";",
            "integer", dt_uint8, 255,
            "packets", dt_const_bindata, "void_ptr", 9ULL,
            "future_value", dt_null,
            "some_object", dt_obj,
            ";",
            "matrix", dt_array, 
                dt_array, 
                    "1", dt_int, 11,
                    "2", dt_int, 22,
                    "3", dt_int, 33,
                ";",
            ";", 
            "some_values", dt_array,
                dt_uint8, 251,
                dt_uint8, 252,
                dt_uint8, 253,
            ";",
            "string", dt_string, "hello_world", 12ULL,
        ";"
    ); 

    // error handling
    if(doc_error_code < 0){
        log("Error: %s",doc_get_error_msg());
        return -1;
    }

    // CRUDAC - Create Read Update Delete Append Copy
    // Get ptr

    doc *ptr = doc_get_ptr(obj, "pontos.p1");
    double value = doc_get(ptr, ".", double);

    // CHECKS
    // check array get() on anonymous members
    doc *array_member = doc_get_ptr(obj, "matrix[0][1]");
    log("Error_check: %s\n",doc_get_error_msg());
    int member_value = doc_get(array_member, ".", int);
    log("Value: [%u]. Error_check: %s\n", member_value, doc_get_error_msg());

    // check [] notation on objs
    doc *obj_member = doc_get_ptr(obj, "pontos[1]");
    log("Error_check: %s\n",doc_get_error_msg());
    double point = doc_get(obj_member, ".", double);
    log("Value: [%f]. Error_check: %s\n", point, doc_get_error_msg());

    // check set_string on non string
    doc_set_string(obj, "integer", "bruhhhh", 8);
    log("Error_check: %s\n",doc_get_error_msg());

    // check set_bindata on non bindata
    doc_set_bindata(obj, "integer", "bruhhhh", 8);
    log("Error_check: %s\n",doc_get_error_msg());

    // check set_value on non value obj
    doc_set(obj, "future_value", double, 255.8);
    log("Error_check: %s\n",doc_get_error_msg());

    // check add of a single value
    doc_add(obj, "pontos", "p5", dt_double, 50.0);
    log("Error_check: %s\n",doc_get_error_msg());

    // check add of a non value
    doc_add(obj, ".", "p_null", dt_null);
    log("Error_check: %s\n",doc_get_error_msg());

    // check add on non addable objects
    doc_add(obj, "pontos", "p5", dt_double, 50.0);
    log("Error_check: %s\n",doc_get_error_msg());

    // check add value on array, with different type
    doc_add(obj, "pontos", "p6", dt_int, 64);
    log("Error_check: %s\n",doc_get_error_msg());

    // check get childs amount
    size_t obj_size = doc_get_childs_amount(obj, "."); 
    log("Error_check: %s\n",doc_get_error_msg());
    log("Obj size: %u\n", obj_size);

    // get string and bindata
    uint8_t *bindata_test = doc_get_bindata(obj, "packets");
    log("Error_check: %s\n",doc_get_error_msg());
    size_t bindata_size = doc_get_bindata_size(obj, "packets");
    log("Error_check: %s\n",doc_get_error_msg());
    log("packets size: %u\n", bindata_size);

    char *string_test = doc_get_string(obj, "string");
    log("Error_check: %s\n",doc_get_error_msg());
    size_t string_size = doc_get_string_len(obj, "string");
    log("Error_check: %s\n",doc_get_error_msg());
    log("string: \"%s\" - size: %u\n", string_test, string_size);

    // copy value variables
    doc *copy = doc_copy(obj, "pontos.p3");
    log("Error_check: %s\n",doc_get_error_msg());
    double copy_value = doc_get(copy, ".", double);
    log("Error_check: %s\n",doc_get_error_msg());
    log("Copy value: %f\n", copy_value);

    // append value variable
    doc_set(copy, ".", double, 999.0);
    log("Error_check: %s\n",doc_get_error_msg());
    // rename check
    doc_rename(copy, ".", "p6");
    log("Error_check: %s\n",doc_get_error_msg());
    doc_append(obj, "pontos", copy);
    log("Error_check: %s\n",doc_get_error_msg());
    double append_value = doc_get(obj, "pontos.p6", double);
    log("Error_check: %s\n",doc_get_error_msg());
    log("Append value: %f\n", append_value);

    // copy obj variables
    doc *copy_obj = doc_copy(obj, "pontos");
    log("Error_check: %s\n",doc_get_error_msg());
    double copy_member = doc_get(copy_obj, "p2", double);
    log("Error_check: %s\n",doc_get_error_msg());
    log("Copied member: %f\n", copy_member);

    // append obj variable
    doc_rename(copy_obj, ".", "pontos_new");
    doc_set(copy_obj, "p2", double, 999.0);
    doc_append(obj, ".", copy_obj);
    log("Error_check: %s\n",doc_get_error_msg());
    double copy_append_value = doc_get(obj, "pontos_new.p2", double);
    log("Error_check: %s\n",doc_get_error_msg());
    log("Copy Append value: %f\n", copy_append_value);

    // check iterator functionality
    for(doc_ite(cursor, obj)){
        log("Member loop: %s\n", cursor->name);
    }

    // interfaces with structs
    struct_ex_t custom_struct = {.value1 = 20, .value2 = 44, .value3 = 69.0};

    doc *struct_doc = doc_struct_new_struct_ex_t(custom_struct);    
    double struct_value = doc_get(struct_doc, "value3", double);
    log("Error_check: %s\n",doc_get_error_msg());
    printf("struct value3: %f\n", struct_value);

    custom_struct.value3 = 75.0;
    doc_struct_set_struct_ex_t(custom_struct, struct_doc);
    struct_value = doc_get(struct_doc, "value3", double);
    log("Error_check: %s\n",doc_get_error_msg());
    printf("struct value3: %f\n", struct_value);

    doc_set(struct_doc, "value2", int , 100);
    doc_struct_get_struct_ex_t(&custom_struct, struct_doc);
    printf("struct value2: %i\n", custom_struct.value2);

    // delete all, but can be any instance
    doc_delete(obj,".");

    return 0;
}
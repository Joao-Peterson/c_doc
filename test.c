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

    double value = doc_get(obj, "pontos.p1", double);

    // CHECKS
    // check array get() on anonymous members
    int member_value = doc_get(obj, "matrix[0][1]", int);
    if(doc_error_code)
        log("[ERROR] Value: [%u]. Error_check: %s\n", member_value, doc_get_error_msg());
    else
        log("[OK]\n");

    // check [] notation on objs
    double point = doc_get(obj, "pontos[1]", double);
    if(doc_error_code)
        log("[ERROR] Value: [%f]. Error_check: %s\n", point, doc_get_error_msg());
    else
        log("[OK]\n");

    // check set_value on non value obj
    doc_set(obj, "future_value", double, 255.8);
    if(doc_error_code == errno_doc_trying_to_set_value_of_non_value_type_data_type)
        log("[OK]\n");
    else
        log("[ERROR] Error_check: %s\n",doc_get_error_msg());

    // check add of a single value
    doc_add(obj, "pontos", "p5", dt_double, 50.0);
    if(doc_error_code)
        log("[ERROR] Error_check: %s\n",doc_get_error_msg());
    else
        log("[OK]\n");

    // check add of a non value
    doc_add(obj, ".", "p_null", dt_null);
    if(doc_error_code)
        log("[ERROR] Error_check: %s\n",doc_get_error_msg());
    else
        log("[OK]\n");

    // check add on non addable objects
    doc_add(obj, "pontos.p5", "extra", dt_bool, true);
    if(doc_error_code == errno_doc_trying_to_add_new_data_to_non_object_or_non_array)
        log("[OK]\n");
    else
        log("[ERROR] Error_check: %s\n",doc_get_error_msg());

    // check add value on array, with different type
    doc_add(obj, "pontos", "p6", dt_int, 64);
    if(doc_error_code == errno_doc_value_not_same_type_as_array)
        log("[OK]\n");
    else
        log("[ERROR] Error_check: %s\n",doc_get_error_msg());

    // check get childs amount
    doc_size_t obj_size = doc_get_size(obj, "."); 
    if(doc_error_code)
        log("[ERROR] Error_check: %s\n", doc_get_error_msg());
    else{
        log("[OK] Obj size: %u\n", obj_size);
        log("[OK]\n");
    }

    // get string and bindata
    uint8_t *bindata_test = doc_get(obj, "packets", uint8_t*);
    if(doc_error_code)
        log("[ERROR] Error_check: %s\n",doc_get_error_msg());
    else
        log("[OK]\n");

    doc_size_t bindata_size = doc_get_size(obj, "packets");
    if(doc_error_code)
        log("[ERROR] Error_check: %s\n",doc_get_error_msg());
    else{
        log("[OK] Packets size: %u\n", bindata_size);
        log("[OK]\n");
    }

    char *string_test = doc_get(obj, "string", char*);
    if(doc_error_code)
        log("[ERROR] Error_check: %s\n",doc_get_error_msg());
    else
        log("[OK]\n");

    doc_size_t string_size = doc_get_size(obj, "string");
    if(doc_error_code)
        log("[ERROR] Error_check: %s\n",doc_get_error_msg());
    else{
        log("[OK] string: \"%s\" - size: %u\n", string_test, string_size);
        log("[OK]\n");
    }

    // copy value variables
    doc *copy = doc_copy(obj, "pontos.p3");
    if(doc_error_code)
        log("[ERROR] Error_check: %s\n",doc_get_error_msg());
    else{
        double copy_value = doc_get(copy, ".", double);
        log("[OK] Error_check: %s\n",doc_get_error_msg());
        log("[OK] Copy value: %f\n", copy_value);
        log("[OK]\n");
    }

    // append value variable
    doc_set(copy, ".", double, 999.0);
    // rename check
    doc_rename(copy, ".", "p6");
    if(doc_error_code)
        log("[ERROR] Error_check: %s\n",doc_get_error_msg());
    else{
        log("[OK]\n");
    }

    doc_append(obj, "pontos", copy);
    if(doc_error_code)
        log("[ERROR] Error_check: %s\n",doc_get_error_msg());
    else{
        double append_value = doc_get(obj, "pontos.p6", double);
        log("[OK] Error_check: %s\n",doc_get_error_msg());
        log("[OK] Append value: %f\n", append_value);
        log("[OK]\n");
    }
    
    // copy obj variables
    doc *copy_obj = doc_copy(obj, "pontos");
    if(doc_error_code)
        log("[ERROR] Error_check: %s\n",doc_get_error_msg());
    else{
        double copy_member = doc_get(copy_obj, "p2", double);
        log("[OK] Error_check: %s\n",doc_get_error_msg());
        log("[OK] Copied member: %f\n", copy_member);
        log("[OK]\n");
    }

    // append obj variable
    doc_rename(copy_obj, ".", "pontos_new");
    doc_set(copy_obj, "p2", double, 999.0);
    doc_append(obj, ".", copy_obj);
    if(doc_error_code)
        log("[ERROR] Error_check: %s\n",doc_get_error_msg());
    else{
        double copy_append_value = doc_get(obj, "pontos_new.p2", double);
        log("[OK] Error_check: %s\n",doc_get_error_msg());
        log("[OK] Copy Append value: %f\n", copy_append_value);
        log("[OK]\n");
    }

    // check iterator functionality
    for(doc_loop(cursor, obj)){
        log("[LOOP] Member loop: %s\n", cursor->name);
    }

    // interfaces with structs
    struct_ex_t custom_struct = {.value1 = 20, .value2 = 44, .value3 = 69.0};

    doc *struct_doc = doc_struct_new_struct_ex_t(custom_struct);    
    if(doc_error_code)
        log("[ERROR] Error_check: %s\n",doc_get_error_msg());
    else{
        double struct_value = doc_get(struct_doc, "value3", double);
        log("[OK] Error_check: %s\n",doc_get_error_msg());
        log("[OK] struct value3: %f\n", struct_value);
        log("[OK]\n");
    }

    custom_struct.value3 = 75.0;
    doc_struct_set_struct_ex_t(custom_struct, struct_doc);
    if(doc_error_code)
        log("[ERROR] Error_check: %s\n",doc_get_error_msg());
    else{
        double struct_value = doc_get(struct_doc, "value3", double);
        log("[OK] Error_check: %s\n",doc_get_error_msg());
        log("[OK] struct value3: %f\n", struct_value);
        log("[OK]\n");
    }
    

    doc_set(struct_doc, "value2", int , 100);
    doc_struct_get_struct_ex_t(&custom_struct, struct_doc);
    if(doc_error_code)
        log("[ERROR] Error_check: %s\n",doc_get_error_msg());
    else{
        log("[OK] struct value2: %i\n", custom_struct.value2);
        log("[OK]\n");
    }

    // delete all, but can be any instance
    doc_delete(obj,".");
    if(doc_error_code)
        log("[ERROR] Error_check: %s\n",doc_get_error_msg());
    else{
        log("[OK]\n");
    }

    return 0;
}
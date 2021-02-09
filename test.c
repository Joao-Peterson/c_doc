#include <stdio.h>
#include <stdlib.h>
#include "doc/doc.h"

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
            "packets", dt_const_bindata, "void_ptr", 9,
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
        ";"
    ); 

    // error handling
    if(doc_error_code < 0){
        printf("Error: %s",doc_get_error_msg());
        return -1;
    }

    // CHECKS

    // check array get() on anonymous members
    doc *array_member = doc_get(obj, "matrix[0][1]");
    printf("Error_check: %s\n",doc_get_error_msg());
    int member_value = doc_get_value(array_member, int);
    printf("Value: [%u]. Error_check: %s\n", member_value, doc_get_error_msg());

    // check [] notation on objs
    doc *obj_member = doc_get(obj, "pontos[1]");
    printf("Error_check: %s\n",doc_get_error_msg());
    double point = doc_get_value(obj_member, double);
    printf("Value: [%f]. Error_check: %s\n", point, doc_get_error_msg());

    // check set_string on non string
    doc_set_string(obj, "integer", "bruhhhh", 8);
    printf("Error_check: %s\n",doc_get_error_msg());

    // check set_bindata on non bindata
    doc_set_bindata(obj, "integer", "bruhhhh", 8);
    printf("Error_check: %s\n",doc_get_error_msg());

    // check set_value on non value obj
    doc_set_value(obj, "future_value", double, 255.8);
    printf("Error_check: %s\n",doc_get_error_msg());

    // check add of a single value
    doc_add(obj, "pontos", "p5", dt_double, 50.0);
    printf("Error_check: %s\n",doc_get_error_msg());

    // check add on non addable objects
    doc_add(obj, "pontos", "p5", dt_double, 50.0);
    printf("Error_check: %s\n",doc_get_error_msg());

    // check add value on array, with different type
    doc_add(obj, "pontos", "p6", dt_int, 64);
    printf("Error_check: %s\n",doc_get_error_msg());

    // delete all, but can be any instance
    doc_delete(obj,".");

    return 0;
}   
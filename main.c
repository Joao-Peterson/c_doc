#include <stdio.h>
#include <stdlib.h>
#include "doc/doc.h"

int main(int argc, char **argv){

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
        ";"
    ); 

    if(obj == NULL){
        printf("Error: %s",doc_get_error_msg());
        return -1;
    }

    doc_add(obj, ".", 
        "medidas", dt_obj,
            "m1", dt_const_string, "bruh", 5,
            "M1", dt_uint32, 35420,
            "data", dt_const_bindata, "AAAAAAAAAAAAAAAAAAA", 20, 
        ";"
    );

    doc *ptr = doc_get(obj, "pontos.p3");

    double value = doc_get_value(ptr, double); 

    printf("Value before: %f.\n", value);

    doc_set_value(obj, "pontos.p3", double, 130.0);

    value = doc_get_value(ptr, double); 

    printf("Value before: %f.\n", value);

    doc_delete(obj,".");

    return 0;
}   
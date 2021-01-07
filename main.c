#include <stdio.h>
#include <stdlib.h>
#include "c_doc/c_doc.h"

int main(int argc, char **argv){

    doc *obj = doc_new_onstack(
        "velocidades", dt_obj,
            "max", dt_double, 23.0,
            "media", dt_double, 22.5,
            "pontos", dt_array,
                "p1", dt_double, 23.0,        
                "p2", dt_double, 23.0,        
                "p3", dt_double, 22.0,        
                "p4", dt_double, 22.0,
            ";",
            "packets", dt_bindata, "len", "void_ptr",
            "future_value", dt_null,
            "some_object",
                "some_other_object", 
                ";",
            ";",
        ";"
    ); 

    double *value1 = doc_get_member(obj, ".max");
    double *value2 = doc_get_member(obj, "pontos.p2");

    // double *value4 = doc_set_member(obj, "pontos", "p2");

    doc_add_member(obj, "some_object.some_other_object",
        "uid", dt_const_string, "v-1234-4321"
    );

    // doc_remove_member();

    doc_free(obj);

    return 0;
}
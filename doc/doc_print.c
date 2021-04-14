#include "doc_print.h"

#define type_to_string(doc_type) #doc_type

const char *doc_type_str_array[] = {
    type_to_string(dt_null),
    type_to_string(dt_obj),
    type_to_string(dt_array),
    type_to_string(dt_double),
    type_to_string(dt_float),
    type_to_string(dt_uint),
    type_to_string(dt_uint64),
    type_to_string(dt_uint32),
    type_to_string(dt_uint16),
    type_to_string(dt_uint8),
    type_to_string(dt_int),
    type_to_string(dt_int64),
    type_to_string(dt_int32),
    type_to_string(dt_int16),
    type_to_string(dt_int8),
    type_to_string(dt_bool),
    type_to_string(dt_string),
    type_to_string(dt_const_string),
    type_to_string(dt_bindata),
    type_to_string(dt_const_bindata)
};

FILE *out = NULL;

fprint_function_t fprint_function = NULL;
print_function_t print_function = NULL;

typedef enum{
    use_fprint,
    use_print
}print_type_t;

print_type_t print_type = 0;

#define print_wrapper(control_string, ...) \
    switch(print_type){ \
        case use_fprint: \
            fprint_function(out, control_string, ##__VA_ARGS__); \
        break; \
        case use_print: \
            print_function(control_string, ##__VA_ARGS__); \
        break; \
    } 

void doc_print_set(print_function_t print_function_ptr){
    print_function = print_function_ptr;
    print_type = use_print;
}

void doc_print_file_set(fprint_function_t fprint_function_ptr, FILE *f_out){
    fprint_function = fprint_function_ptr;
    out = f_out;
    print_type = use_fprint;
}

void print_variable(doc *variable, size_t level){
    for(size_t i = 0; i < level; i++)
        print_wrapper("    ");
    
    switch(variable->type){
        case dt_obj:
        case dt_array:
            print_wrapper("[%s] (%s): \n", variable->name, doc_type_str_array[variable->type]);

            for(doc_ite(member, variable)){
                print_variable(member, level + 1);
            }
        break;

        case dt_null:
            print_wrapper("[%s] (%s): \"null\"\n", variable->name, doc_type_str_array[variable->type]);
        break;
        
        case dt_double:
            print_wrapper("[%s] (%s): \"%f\"\n", variable->name, doc_type_str_array[variable->type], ((doc_double*)variable)->value);
        break;
        case dt_float:
            print_wrapper("[%s] (%s): \"%f\"\n", variable->name, doc_type_str_array[variable->type], ((doc_float*)variable)->value);
        break;

        case dt_bool:
            print_wrapper("[%s] (%s): \"%s\"\n", variable->name, doc_type_str_array[variable->type], (((doc_bool*)variable)->value) ? "true" : "false");
        break;
        case dt_uint:
            print_wrapper("[%s] (%s): \"%u\"\n", variable->name, doc_type_str_array[variable->type], ((doc_uint_t*)variable)->value);
        break;
        case dt_uint64:
            print_wrapper("[%s] (%s): \"%u\"\n", variable->name, doc_type_str_array[variable->type], ((doc_uint64_t*)variable)->value);
        break;
        case dt_uint32:
            print_wrapper("[%s] (%s): \"%u\"\n", variable->name, doc_type_str_array[variable->type], ((doc_uint32_t*)variable)->value);
        break;
        case dt_uint16:
            print_wrapper("[%s] (%s): \"%u\"\n", variable->name, doc_type_str_array[variable->type], ((doc_uint16_t*)variable)->value);
        break;
        case dt_uint8:
            print_wrapper("[%s] (%s): \"%u\"\n", variable->name, doc_type_str_array[variable->type], ((doc_uint8_t*)variable)->value);
        break;
        case dt_int:
            print_wrapper("[%s] (%s): \"%i\"\n", variable->name, doc_type_str_array[variable->type], ((doc_int*)variable)->value);
        break;
        case dt_int64:
            print_wrapper("[%s] (%s): \"%i\"\n", variable->name, doc_type_str_array[variable->type], ((doc_int64_t*)variable)->value);
        break;
        case dt_int32:
            print_wrapper("[%s] (%s): \"%i\"\n", variable->name, doc_type_str_array[variable->type], ((doc_int32_t*)variable)->value);
        break;
        case dt_int16:
            print_wrapper("[%s] (%s): \"%i\"\n", variable->name, doc_type_str_array[variable->type], ((doc_int16_t*)variable)->value);
        break;
        case dt_int8:
            print_wrapper("[%s] (%s): \"%i\"\n", variable->name, doc_type_str_array[variable->type], ((doc_int8_t*)variable)->value);
        break;
        
        case dt_string:
        case dt_const_string:
            print_wrapper("[%s] (%s): \"%s\"\n", variable->name, doc_type_str_array[variable->type], ((doc_string*)variable)->string);
        break;

        case dt_bindata:
        case dt_const_bindata:        
            print_wrapper("[%s] (%s): \"0x\"", variable->name, doc_type_str_array[variable->type]);

            for(doc_size_t j = 0; j < ((doc_bindata*)variable)->len; j++)
                print_wrapper("%X", ((doc_bindata*)variable)->data[j]);
                
            print_wrapper("\n");
        break;
    }
}

void doc_print(doc *variable){
    if(print_type == 0){
        out = stdout;
        print_function = printf;
        print_type = use_print;
    } 
    
    print_variable(variable, 0);
}
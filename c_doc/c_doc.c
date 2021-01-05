#include "c_doc.h"
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include "static_assert/static_assert.h"

/* ----------------------------------------- Private Functions ------------------------------ */

doc *parse_doc_syntax(char *name, doc_type_t type, va_list arg_list){

    assert(IS_DOC_TYPE(type) && "Type_provided_is_not_an_actual_type!");
    doc *return_var = NULL;

    switch(type){

        case dt_array:
        case dt_obj:

            doc *variable = calloc(1, sizeof(doc));
            return_var = variable;
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;

            doc *last_doc;
            doc *current_doc;
            char *member_name;
            doc_type_t member_type;
            doc_type_t array_type_check;

            // first member
            // pick name(key) and type
            member_name = va_arg(arg_list, char*);
            if(member_name[0] == ';') // if obj has no members then quit
                break;
            member_type = va_arg(arg_list,doc_type_t);
            array_type_check = member_type;
            assert(IS_DOC_TYPE(member_type) && "Type_provided_is_not_an_actual_type!");
            
            // parse first the one
            current_doc = parse_doc_syntax(member_name, member_type, arg_list);

            current_doc->prev = NULL;
            current_doc->next = NULL;
            // the first one is the head of the linked list of members and child of the object
            variable->child = current_doc;
            last_doc = current_doc; 

            // read next arg
            member_name = va_arg(arg_list, char*);
            member_type = va_arg(arg_list,doc_type_t);
            assert(IS_DOC_TYPE(member_type) && "Type_provided_is_not_an_actual_type!");
            
            while(member_name[0] != ';'){ // wait until ";" to close object    
                // parse
                current_doc = parse_doc_syntax(member_name, member_type, arg_list);
                // assign to linked list
                last_doc->next = current_doc;
                current_doc->prev = last_doc;
                current_doc->next = NULL;
                last_doc = current_doc;

                // read next
                member_name = va_arg(arg_list, char*);
                member_type = va_arg(arg_list,doc_type_t);
                assert(IS_DOC_TYPE(member_type) && "Type_provided_is_not_an_actual_type!");
                // if the object is an array, assert so that all members have the same type, for correctness
                if(variable->type == dt_array)
                    assert(type == array_type_check && "All_values_in_array_must_be_of_the_same_type!");
            }

        break;
        
        case dt_int:
            doc *variable = calloc(1,sizeof(doc_int));
            return_var = variable;
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_int*)variable)->value = va_arg(arg_list, int); 
        break;
        
        case dt_int64:
            doc *variable = calloc(1,sizeof(doc_int64));
            return_var = variable;
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_int64*)variable)->value = va_arg(arg_list, int64_t); 
        break;
        
        case dt_int32:
            doc *variable = calloc(1,sizeof(doc_int32));
            return_var = variable;
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_int32*)variable)->value = va_arg(arg_list, int32_t); 
        break;
        
        case dt_int16:
            doc *variable = calloc(1,sizeof(doc_int16));
            return_var = variable;
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_int16*)variable)->value = va_arg(arg_list, int16_t); 
        break;
        
        case dt_int8:
            doc *variable = calloc(1,sizeof(doc_int8));
            return_var = variable;
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_int8*)variable)->value = va_arg(arg_list, int8_t); 
        break;
        
        case dt_uint:
            doc *variable = calloc(1,sizeof(doc_uint));
            return_var = variable;
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_uint*)variable)->value = va_arg(arg_list, unsigned int); 
        break;
        
        case dt_uint64:
            doc *variable = calloc(1,sizeof(doc_uint64));
            return_var = variable;
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_uint64*)variable)->value = va_arg(arg_list, uint64_t); 
        break;
        
        case dt_uint32:
            doc *variable = calloc(1,sizeof(doc_uint32));
            return_var = variable;
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_uint32*)variable)->value = va_arg(arg_list, uint32_t); 
        break;
        
        case dt_uint16:
            doc *variable = calloc(1,sizeof(doc_uint16));
            return_var = variable;
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_uint16*)variable)->value = va_arg(arg_list, uint16_t); 
        break;
        
        case dt_uint8:
            doc *variable = calloc(1,sizeof(doc_uint8));
            return_var = variable;
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_uint8*)variable)->value = va_arg(arg_list, uint8_t); 
        break;

        case dt_bool:
            doc *variable = calloc(1,sizeof(doc_bool));
            return_var = variable;
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_bool*)variable)->value = va_arg(arg_list, bool); 
        break;

        case dt_float:
            doc *variable = calloc(1,sizeof(doc_float));
            return_var = variable;
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_float*)variable)->value = va_arg(arg_list, float); 
        break;

        case dt_double:
            doc *variable = calloc(1,sizeof(doc_double));
            return_var = variable;
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_double*)variable)->value = va_arg(arg_list, double); 
        break;

        case dt_string:
        case dt_const_string:
        case dt_bindata:
        case dt_const_bindata:
            doc *variable = calloc(1,sizeof(doc_bin_data)); // for strings or bin_data, the allocation doesn't matter, just the type variable being of the correct type
            return_var = variable;
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            // order of assignement to len and data matters, should be the same as in the struct declaration
            ((doc_bin_data*)variable)->len  = va_arg(arg_list, size_t); 
            ((doc_bin_data*)variable)->data = va_arg(arg_list, uint8_t*); 
        break;

        case dt_null:
            doc *variable = calloc(1,sizeof(doc));
            return_var = variable;
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
        break;

        default:
            // trown error
        break;
    }

    return return_var;
}

void delete_doc(doc *variable){
    switch(variable->type){
        case dt_obj:
        case dt_array:

        break;

        case dt_null:
        case dt_double:
        case dt_float:
        case dt_uint:
        case dt_uint64:
        case dt_uint32:
        case dt_uint16:
        case dt_uint8:
        case dt_int:
        case dt_int64:
        case dt_int32:
        case dt_int16:
        case dt_int8:
        case dt_bool:
            free(variable);
        break;

        case dt_string:
        case dt_bindata:
            free(((doc_bin_data*)variable)->data);
            free(variable);
        break;
        
        case dt_const_string:
        case dt_const_bindata:

        break;
    }
}

/* ----------------------------------------- Functions -------------------------------------- */

doc *doc_new_onstack(char *name, doc_type_t type, ...){
    va_list args;
    va_start(args, type);

    doc *variable = parse_doc_syntax(name, type, args);

    va_end(args);

    return variable;
}

void doc_add_member(doc *obj_or_array, char *name, doc_type_t type, ...){
    va_list args;
    va_start(args, type);

    doc *variable = parse_doc_syntax(name, type, args);

    doc *cursor = obj_or_array->child;

    while(cursor->next != NULL)
        cursor = cursor->next;

    cursor->next = variable;
    variable->prev = cursor;

    va_end(args);
}

void doc_remove_member(doc *object_or_array, char *name){
    doc *cursor = object_or_array->child;

    while(cursor->next != NULL){
        if(!strcmp(name,cursor->name)){
            
        }
    }

    return;
}

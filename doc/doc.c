#include "doc.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

/* ----------------------------------------- Private definitions ------------------------------ */

#define ERR_TO_STRUCT(err)  {err, #err} 

/* ----------------------------------------- Private Enum's ----------------------------------- */

// error codes
typedef enum{
    errno_doc_ok                            =  0,
    errno_doc_not_a_type                    = -1,
    errno_doc_no_value                      = -2,
    errno_doc_value_not_same_type_as_array  = -3,
    errno_doc_no_type_specified             = -4,
    errno_doc_missing_terminator            = -5,
    errno_doc_duplicate_names               = -6,
    errno_doc_null_passed_obj               = -7,
    errno_doc_obj_not_found                 = -8
}errno_doc_code_t;

/* ----------------------------------------- Private Struct's --------------------------------- */

// error struct for ease error handling 
typedef struct{
    errno_doc_code_t code;
    char *msg;
}errno_doc_t;

/* ----------------------------------------- Private Globals -------------------------------- */

// internal error vars, the char * one holds information about the name of instance to be acted on
errno_doc_code_t errno_doc_code_internal = 0;
char *errno_msg_doc_internal = NULL;

// array to get the value and name of defined errors 
const errno_doc_t errno_doc_msg_code_array[] = {
    ERR_TO_STRUCT(errno_doc_ok),
    ERR_TO_STRUCT(errno_doc_not_a_type),
    ERR_TO_STRUCT(errno_doc_no_value),
    ERR_TO_STRUCT(errno_doc_value_not_same_type_as_array),
    ERR_TO_STRUCT(errno_doc_no_type_specified),
    ERR_TO_STRUCT(errno_doc_missing_terminator),
    ERR_TO_STRUCT(errno_doc_duplicate_names),
    ERR_TO_STRUCT(errno_doc_null_passed_obj),
    ERR_TO_STRUCT(errno_doc_obj_not_found)
};

/* ----------------------------------------- Private Functions ------------------------------ */

// check instance for name duplicates
int is_name_duplicate(doc *obj_or_array, char *name){
    doc *cursor = obj_or_array->child;
    if(cursor == NULL)
        return -1; // empty object

    do{
        if(!strcmp(cursor->name, name))
            return 1; // is duplicate

        cursor = cursor->next;
    }while(cursor != NULL);
        
    return 0;
}

// parse the syntax and build the data structure recursevily
doc *parse_doc_syntax(char *name, doc_type_t type, va_list *arg_list){

    if(!IS_DOC_TYPE(type)){
        errno_msg_doc_internal = name;
        errno_doc_code_internal = errno_doc_not_a_type;
        return NULL;
    }

    doc *variable = NULL;

    switch(type){

        case dt_array:
        case dt_obj:

            variable = calloc(1, sizeof(doc));
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->parent = NULL;
            variable->prev = NULL;
            variable->next = NULL;

            doc *last_doc = NULL;
            doc *current_doc;
            char *member_name;
            doc_type_t member_type;
            doc_type_t array_type_check;

            for(int i = 0; true; i++){                                              // loop trought

                member_name = va_arg(*arg_list, char *);                            // MEMBER NAME

                if(member_name[0] == ';')                                           // if obj has no members then quit
                    break;

                if(i != 0){                                                         // on all members execept the first
                    if(is_name_duplicate(variable, member_name)){                  // check for duplicate names
                        errno_doc_code_internal = errno_doc_duplicate_names;
                        errno_msg_doc_internal = member_name;
                        return NULL;
                    }
                }

                member_type = va_arg(*arg_list, doc_type_t);                        // MEMBER TYPE

                if(!IS_DOC_TYPE(member_type)){                                      // check type validity
                    errno_msg_doc_internal = member_name;
                    errno_doc_code_internal = errno_doc_not_a_type;
                    return NULL;
                }

                // check for type correctness on array type members
                if( (variable->type == dt_array) && (i != 0) && (member_type != array_type_check) ){  
                    errno_doc_code_internal = errno_doc_value_not_same_type_as_array;
                    errno_msg_doc_internal = member_name;
                    return NULL;
                }
                array_type_check = member_type;

                doc *member = parse_doc_syntax(member_name, member_type, arg_list); // PARSE THE MEMBER 
                if(member == NULL)
                    return NULL;


                if(i == 0){
                    member->prev = NULL;                                            // if first, prev link is NULL
                    variable->child = member;                                       // Put into data structure 
                }
                else{
                    member->prev = last_doc;                                        
                    last_doc->next = member;
                }
                
                member->next = NULL;
                member->parent = variable;
                last_doc = member;
            }

        break;
        
        case dt_int:
            variable = calloc(1,sizeof(doc_int_t));
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_int_t*)variable)->value = va_arg(*arg_list, int); 
        break;
        
        case dt_int64:
            variable = calloc(1,sizeof(doc_int64_t));
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_int64_t*)variable)->value = va_arg(*arg_list, int64_t); 
        break;
        
        case dt_int32:
            variable = calloc(1,sizeof(doc_int32_t));
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_int32_t*)variable)->value = va_arg(*arg_list, int); 
        break;
        
        case dt_int16:
            variable = calloc(1,sizeof(doc_int16_t));
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_int16_t*)variable)->value = va_arg(*arg_list, int); 
        break;
        
        case dt_int8:
            variable = calloc(1,sizeof(doc_int8_t));
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_int8_t*)variable)->value = va_arg(*arg_list, int); 
        break;
        
        case dt_uint:
            variable = calloc(1,sizeof(doc_uint_t));
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_uint_t*)variable)->value = va_arg(*arg_list, unsigned int); 
        break;
        
        case dt_uint64:
            variable = calloc(1,sizeof(doc_uint64_t));
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_uint64_t*)variable)->value = va_arg(*arg_list, uint64_t); 
        break;
        
        case dt_uint32:
            variable = calloc(1,sizeof(doc_uint32_t));
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_uint32_t*)variable)->value = va_arg(*arg_list, unsigned int); 
        break;
        
        case dt_uint16:
            variable = calloc(1,sizeof(doc_uint16_t));
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_uint16_t*)variable)->value = va_arg(*arg_list, unsigned int); 
        break;
        
        case dt_uint8:
            variable = calloc(1,sizeof(doc_uint8_t));
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_uint8_t*)variable)->value = va_arg(*arg_list, unsigned int); 
        break;

        case dt_bool:
            variable = calloc(1,sizeof(doc_bool));
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_bool*)variable)->value = va_arg(*arg_list, int); 
        break;

        case dt_float:
            variable = calloc(1,sizeof(doc_float));
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            ((doc_float*)variable)->value = va_arg(*arg_list, double); 
        break;

        case dt_double:
            variable = calloc(1,sizeof(doc_double));
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            double deg = va_arg(*arg_list, double);
            ((doc_double*)variable)->value = deg; 
        break;

        case dt_string:
        case dt_const_string:
        case dt_bindata:
        case dt_const_bindata:
            variable = calloc(1,sizeof(doc_bindata)); // for strings or bin_data, the allocation doesn't matter, just the type variable being of the correct type
            variable->name = name;
            variable->type = type;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;
            // order of assignement to len and data matters, should be the same as in the struct declaration
            ((doc_bindata*)variable)->data = va_arg(*arg_list, uint8_t*); 
            ((doc_bindata*)variable)->len  = va_arg(*arg_list, size_t); 
        break;

        case dt_null:
            variable = calloc(1,sizeof(doc));
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

    return variable;
}

// get pointer to instance
doc *get_variable_ptr(doc *object_or_array, char *path){
    
    doc *cursor = object_or_array->child;                                           // to see if it contains at least one member
    if(cursor == NULL)
        return NULL;

    if(!strcmp(path,"."))                                                           // the element itself
        return object_or_array;

    if(path[0] == '.'){ path++; }                                                   // jump over optional '.' at the beginning of path
    char *var_name = path;
    char *var_name_next = strpbrk(var_name, ".");

    if(var_name_next != NULL){                                                      // on the last one
        var_name_next[0] = '\0';                                                    // make the first name acessible directly
        var_name_next++;                                                            // points to other name
    }

    do{                                                                             // search names
        if(!strcmp(cursor->name,var_name)){
            if(var_name_next == NULL)
                return cursor;                                                      // if there are no elements left, return this one
            else
                return get_variable_ptr(cursor, var_name_next);                     // elese, call the function again on the left elements
        }

        cursor = cursor->next;
    }while(cursor != NULL);

    return NULL;
}

/* ----------------------------------------- Functions -------------------------------------- */

// get error code 
int _MACRO_WANNABE_doc_get_error_code(void){
    return errno_doc_code_internal;
}

char *doc_get_error_msg(void){
    char *message = malloc(ERROR_MSG_LEN_DOC_HEADER);
    message[ERROR_MSG_LEN_DOC_HEADER] = '\0';
    char *error_msg = NULL;

    // get errors array size
    size_t esp_doc_msg_code_array_size = sizeof(errno_doc_msg_code_array)/sizeof(errno_doc_msg_code_array[0]);

    // get error name message
    for(int i = 0; i < esp_doc_msg_code_array_size; i++){
        if(errno_doc_msg_code_array[i].code == errno_doc_code_internal)
            error_msg = errno_doc_msg_code_array[i].msg;
    }

    // print
    if(errno_doc_code_internal == errno_doc_ok){
        snprintf(message, ERROR_MSG_LEN_DOC_HEADER - 1, "%s.", error_msg);
    }
    else{
        snprintf(message, ERROR_MSG_LEN_DOC_HEADER - 1, "%s. Instance: \"%s\".", error_msg, errno_msg_doc_internal);
    }

    return message;
}

doc *doc_new(char *name, doc_type_t type, ...){
    va_list args;
    va_start(args, type);

    doc *variable = parse_doc_syntax(name, type, &args);

    va_end(args);

    return variable;
}

void doc_add(doc *object_or_array, char *name_to_add_to, char *name, doc_type_t type, ...){

    char *name_cpy = malloc(strlen(name_to_add_to)+1);                                        // copy for use in tokenization (destructive)
    strcpy(name_cpy, name_to_add_to);

    doc *variable = get_variable_ptr(object_or_array, name_cpy);                    // get instance from name

    free(name_cpy);
    
    if(variable == NULL)                                                            // no instance of name found, return
        return;

    va_list args;
    va_start(args, type);

    doc *new_variable = parse_doc_syntax(name, type, &args);
    if(new_variable == NULL)
        return

    va_end(args);

    if(variable->child == NULL){
        variable->child = new_variable;
    }
    else{
        variable = variable->child;
        while(variable->next != NULL)
            variable = variable->next;

        variable->next = new_variable;
        new_variable->prev = variable;
    }

    return;
}

void doc_delete(doc *object_or_array, char *name){
    
    char *name_cpy = malloc(strlen(name)+1);                                        // copy for use in tokenization (destructive)
    strcpy(name_cpy, name);

    doc *variable = get_variable_ptr(object_or_array, name_cpy);                    // get pointer to instance
    if(variable == NULL){
        errno_doc_code_internal = errno_doc_null_passed_obj;
        errno_msg_doc_internal = name;
        return;
    }

    free(name_cpy);

    switch(variable->type){                                                     
        case dt_array:                                                              // in case of a entire OBJ OR ARRAY
        case dt_obj:
            while(variable->child == NULL){
                doc_delete(variable->child, ".");
            }
            
            free(variable);
        break;

        default:                                                                    // in case of a VALUE element

            if(variable->prev == NULL && variable->next == NULL){                   // last elemen of obj or array 
                variable->parent->child = NULL;
            }
            else if(variable->parent->child == variable){                           // first element of an object or array
                variable->parent->child = variable->next;
                variable->next->prev = NULL;
            }
            else if(variable->next == NULL){                                        // last element of an object or array
                variable->prev->next = NULL;
            }
            else{                                                                   // an element of a object or array
                variable->prev->next = variable->next;
                variable->next->prev = variable->prev;
            }

            switch(variable->type){                                                 // if non const type of string and bindata, free its content as well
                case dt_string:
                    free(((doc_string*)variable)->string);
                    free(variable);                    
                break;

                case dt_bindata:
                    free(((doc_bindata*)variable)->data);
                    free(variable);                    
                break;

                default:
                    free(variable);
                break;
            }

        break;
    }
}

doc* doc_get(doc* object_or_array, char *name){
    // check to see if doc* is obj/array
    if(object_or_array->type != dt_obj && object_or_array->type != dt_array){
        errno_doc_code_internal = errno_doc_null_passed_obj;
        errno_msg_doc_internal = "null";
        return NULL;
    }

    // copy for use in tokenization (destructive)
    char *name_cpy = malloc(strlen(name)+1);
    strcpy(name_cpy, name);

    doc *variable = get_variable_ptr(object_or_array, name_cpy);

    if(variable == NULL){
        errno_doc_code_internal = errno_doc_obj_not_found;
        errno_msg_doc_internal  = name;
        return NULL; 
    }

    free(name_cpy);

    return variable;
}

/*
// TODO:
// Somente arrays e objetos podem ter "childs", programar verificação
// Adicionar mais checagem de erro, erros já cadastrados no typedef e outros para outras operação como, add, delete, modify e etc.
*/
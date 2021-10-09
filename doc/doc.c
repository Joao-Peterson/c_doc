/**
 * @file doc.c
 * 
 * DOC - The C dynamic object lib!
 * 
 * This library is made for C and C++ programs and implement a data structure to mimic dynamic objects
 * present in interpreted languages, parsers can be written to convert files notations such as xml, json, etc, to
 * the data structure.
 * 
 * Created by: João Peterson Scheffer - 2021.
 * 
 * This is the source file, please don't modify, except when stated in documentation.
 * 
 */

#include "doc.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include "parse_utils.h"

/* ----------------------------------------- Private definitions ------------------------------ */

#define ERR_TO_STRUCT(err)  {err, #err} 

/* ----------------------------------------- Private Struct's --------------------------------- */

// error struct for ease error handling 
typedef struct{
    errno_doc_code_t code;
    char *msg;
}errno_doc_t;

/* ----------------------------------------- Private Globals -------------------------------- */

// dummy instance to receive macros operations, to not generate segfault
static doc_uint64_t dummy_doc_internal = { .value = 0xFFFFFFFFFFFFFFFF, .header = { .name = "dummy", .type = dt_null, .parent = NULL, .child = NULL, .next = NULL, .prev = NULL, } };
static doc *dummy_doc_internal_ptr = (doc*)&dummy_doc_internal;

// ilegal ascii characters on names
static const char *illegal_chars_doc_name = "\a\b\t\n\v\f\r\"\'()*+,.\\";

// internal error vars, the char * one holds information about the name of instance to be acted on
static errno_doc_code_t errno_doc_code_internal = 0;
static char *errno_msg_doc_internal = NULL;

// array to get the value and name of defined errors 
static const errno_doc_t errno_doc_msg_code_array[] = {
    ERR_TO_STRUCT(errno_doc_size_of_string_or_bindata_is_beyond_four_megabytes_Check_if_size_is_of_type_size_t_or_cast_it_to_size_t_first),
    ERR_TO_STRUCT(errno_doc_ok),
    ERR_TO_STRUCT(errno_doc_not_a_type),
    ERR_TO_STRUCT(errno_doc_overflow_quantity_members_or_name_is_too_big),
    ERR_TO_STRUCT(errno_doc_value_not_same_type_as_array),
    ERR_TO_STRUCT(errno_doc_duplicate_names),
    ERR_TO_STRUCT(errno_doc_null_passed_doc_ptr),
    ERR_TO_STRUCT(errno_doc_value_not_found),
    ERR_TO_STRUCT(errno_doc_name_cointains_illegal_characters_or_missing_semi_colon_terminator),
    ERR_TO_STRUCT(errno_doc_trying_to_add_new_data_to_non_object_or_non_array),
    ERR_TO_STRUCT(errno_doc_trying_to_get_data_from_non_object_or_non_array),
    ERR_TO_STRUCT(errno_doc_trying_to_set_value_of_non_value_type_data_type),
    ERR_TO_STRUCT(errno_doc_trying_to_set_string_of_non_string_data_type),
    ERR_TO_STRUCT(errno_doc_trying_to_set_bindata_of_non_bindata_data_type),
    ERR_TO_STRUCT(errno_doc_trying_to_squash_a_doc_structure_to_0__This_is_not_possible_becaue_it_needs_at_least_one_object_to_hold_data),
    ERR_TO_STRUCT(errno_doc_null_passed_parameter)
};

/* ----------------------------------------- Private Functions ------------------------------ */

// allocate and copy data
static void *_mem_alloc_cpy(void *data, size_t size, size_t elements){
    void *buffer = calloc(1, size*elements);
    memcpy(buffer, data, size*elements);
    return buffer;
}

// check instance for name duplicates
static int is_name_duplicate(doc *obj_or_array, char *name){
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
static doc *parse_doc_syntax(char *name, doc_type_t type, va_list *arg_list){
    if(!IS_DOC_TYPE(type)){
        errno_msg_doc_internal = name;
        errno_doc_code_internal = errno_doc_not_a_type;
        return NULL;
    }

    if(name != NULL){
        char *char_veri = strpbrk(name,illegal_chars_doc_name);
        if( char_veri != NULL){
            errno_doc_code_internal = errno_doc_name_cointains_illegal_characters_or_missing_semi_colon_terminator;
            errno_msg_doc_internal = name;
            return NULL;
        }
    }
    else{
        name = "";
    }

    uint8_t *buffer;
    doc *variable = NULL;

    char *name_copy = malloc(sizeof(*name_copy)*(strlen(name) + 1));
    memcpy(name_copy, name, strlen(name) + 1);

    switch(type){

        case dt_array:
        case dt_obj:

            variable = calloc(1, sizeof(doc));
            variable->name = name_copy;
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
            doc_size_t i;

            for(i = 0; true; i++){                                                  // loop trought

                member_name = va_arg(*arg_list, char *);                            // MEMBER NAME

                doc_type_t name_check_for_type = *((doc_type_t*)&member_name);

                if(variable->type == dt_array && IS_DOC_TYPE(name_check_for_type) ){// in case of an array, members of type array and obj can have a null name  

                    member_type = *((doc_type_t*)&member_name);                     // MEMBER TYPE
                    member_name = malloc(sizeof(char*)*1);
                    strncpy(member_name, "", 1);

                }
                else{

                    if(i > MAX_OBJ_MEMBER_QTY || strlen(member_name) > DOC_NAME_MAX_LEN){
                        errno_doc_code_internal = errno_doc_overflow_quantity_members_or_name_is_too_big;
                        errno_msg_doc_internal = variable->name;
                        return NULL;
                    }

                    if(member_name[0] == ';')                                       // quit when terminator char found
                        break;

                    if(i != 0){                                                     // on all members execept the first
                        if(is_name_duplicate(variable, member_name)){               // check for duplicate names
                            errno_doc_code_internal = errno_doc_duplicate_names;
                            errno_msg_doc_internal = member_name;
                            return NULL;
                        }
                    }

                    member_type = va_arg(*arg_list, doc_type_t);                    // MEMBER TYPE

                }

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

            variable->childs = i;

        break;
        
        default:
            switch(type){

                case dt_int:
                    variable = calloc(1,sizeof(doc_int));
                    ((doc_int*)variable)->value = va_arg(*arg_list, int); 
                break;
                
                case dt_int64:
                    variable = calloc(1,sizeof(doc_int64_t));
                    ((doc_int64_t*)variable)->value = va_arg(*arg_list, int64_t); 
                break;
                
                case dt_int32:
                    variable = calloc(1,sizeof(doc_int32_t));
                    ((doc_int32_t*)variable)->value = va_arg(*arg_list, int); 
                break;
                
                case dt_int16:
                    variable = calloc(1,sizeof(doc_int16_t));
                    ((doc_int16_t*)variable)->value = va_arg(*arg_list, int); 
                break;
                
                case dt_int8:
                    variable = calloc(1,sizeof(doc_int8_t));
                    ((doc_int8_t*)variable)->value = va_arg(*arg_list, int); 
                break;
                
                case dt_uint:
                    variable = calloc(1,sizeof(doc_uint_t));
                    ((doc_uint_t*)variable)->value = va_arg(*arg_list, unsigned int); 
                break;
                
                case dt_uint64:
                    variable = calloc(1,sizeof(doc_uint64_t));
                    ((doc_uint64_t*)variable)->value = va_arg(*arg_list, uint64_t); 
                break;
                
                case dt_uint32:
                    variable = calloc(1,sizeof(doc_uint32_t));
                    ((doc_uint32_t*)variable)->value = va_arg(*arg_list, unsigned int); 
                break;
                
                case dt_uint16:
                    variable = calloc(1,sizeof(doc_uint16_t));
                    ((doc_uint16_t*)variable)->value = va_arg(*arg_list, unsigned int); 
                break;
                
                case dt_uint8:
                    variable = calloc(1,sizeof(doc_uint8_t));
                    ((doc_uint8_t*)variable)->value = va_arg(*arg_list, unsigned int); 
                break;

                case dt_bool:
                    variable = calloc(1,sizeof(doc_bool));
                    ((doc_bool*)variable)->value = va_arg(*arg_list, int); 
                break;

                case dt_float:
                    variable = calloc(1,sizeof(doc_float));
                    ((doc_float*)variable)->value = va_arg(*arg_list, double); 
                break;

                case dt_double:
                    variable = calloc(1,sizeof(doc_double));
                    double deg = va_arg(*arg_list, double);
                    ((doc_double*)variable)->value = deg; 
                break;

                case dt_const_string:
                case dt_const_bindata:
                    variable = calloc(1,sizeof(doc_bindata)); // for strings or bin_data, the allocation doesn't matter, just the type variable being of the correct type
                    // order of assignement to len and data matters, should be the same as in the struct declaration
                    ((doc_bindata*)variable)->data = va_arg(*arg_list, uint8_t*);
                    ((doc_bindata*)variable)->len = va_arg(*arg_list, size_t); 

                    if(((doc_bindata*)variable)->len > 0x0000000100000000){
                        errno_doc_code_internal = errno_doc_size_of_string_or_bindata_is_beyond_four_megabytes_Check_if_size_is_of_type_size_t_or_cast_it_to_size_t_first;
                        errno_msg_doc_internal = name;
                    }
                break;
                
                case dt_string:
                case dt_bindata:
                    variable = calloc(1,sizeof(doc_bindata)); // for strings or bin_data, the allocation doesn't matter, just the type variable being of the correct type
                    // order of assignement to len and data matters, should be the same as in the struct declaration
                    buffer = va_arg(*arg_list, uint8_t*); 
                    ((doc_bindata*)variable)->len = va_arg(*arg_list, size_t); 
                    
                    ((doc_bindata*)variable)->data = calloc(((doc_bindata*)variable)->len + 1, sizeof(uint8_t)); 
                    memcpy(((doc_bindata*)variable)->data, buffer, ((doc_bindata*)variable)->len + 1);

                    if(((doc_bindata*)variable)->len > 0x0000000100000000){
                        errno_doc_code_internal = errno_doc_size_of_string_or_bindata_is_beyond_four_megabytes_Check_if_size_is_of_type_size_t_or_cast_it_to_size_t_first;
                        errno_msg_doc_internal = name;
                    }
                break;

                case dt_null:
                    variable = calloc(1,sizeof(doc));
                break;
            }

            variable->name = name_copy;
            variable->type = type;
            variable->childs = 0;
            variable->child = NULL;
            variable->prev = NULL;
            variable->next = NULL;

        break;
    }

    return variable;
}

// get pointer to instance
static doc *get_variable_ptr(doc *object_or_array, char *path){
    
    if(object_or_array == NULL){
        errno_doc_code_internal = errno_doc_null_passed_doc_ptr;
        errno_msg_doc_internal = path;
        return NULL;
    }

    char *name_cpy = malloc(strlen(path)+1);                                        // copy for use in tokenization (destructive)
    char *name_cpy_base = name_cpy;
    strcpy(name_cpy, path);

    if(!strcmp(name_cpy,"."))                                                       // the element itself
        return object_or_array;

    doc *cursor = object_or_array->child;                                           // to see if it contains at least one member
    if(cursor == NULL)
        return NULL;

    if(name_cpy[0] == '.') name_cpy++;                                              // jump over optional '.' at the beginning of path
    char *var_name = name_cpy;
    char *var_name_next = strpbrk(var_name, ".[");

    if(var_name_next != NULL){                                                      // on the last one
        var_name_next[0] = '\0';                                                    // make the first name acessible directly
        var_name_next++;                                                            // points to other name
    }
    
    if(var_name[1] == ']' && ( var_name[0] >= '0' && var_name[0] <= '9' ) ){        // search by index number
    
        doc_size_t index = strtoull(var_name, NULL, 10);

        if(index > object_or_array->childs){
            free(name_cpy_base);
            return NULL;
        }

        for(doc_size_t i = 0ULL; i < index; i++)
            cursor = cursor->next;

        if(var_name_next == NULL){
            free(name_cpy_base);
            return cursor;                                                  
        }
        else{
            doc *return_ptr = get_variable_ptr(cursor, var_name_next);      
            free(name_cpy_base);
            return return_ptr;
        }
    }
    else if(*var_name == '\0'){                                                     // if name is empty, try again on same variable
        return get_variable_ptr(object_or_array, var_name_next);
    }
    else{

        do{                                                                         // search by name                                      
            if(!strcmp(cursor->name,var_name)){
                if(var_name_next == NULL){
                    free(name_cpy_base);
                    return cursor;                                                  
                }
                else{
                    doc *return_ptr = get_variable_ptr(cursor, var_name_next);      
                    free(name_cpy_base);
                    return return_ptr;
                }
            }

            cursor = cursor->next;
        }while(cursor != NULL);
    }

    free(name_cpy_base);
    return NULL;
}

// recusive doc squash call
static doc *squash(doc *variable, doc_size_t max_depth, doc_size_t depth){
    if(variable == NULL) return NULL;
    
    switch(variable->type){
        case dt_array:                                                              // call squash on childsn then delet itself
        case dt_obj:
        
            for(doc_loop(member, variable)){
                member = squash(member, max_depth, depth + 1);
            }

            doc *rtn = variable;

            if(depth >= max_depth){
                doc *last = variable->child;

                while(last->next != NULL){
                    last->parent = variable->parent;
                    variable->parent->childs++;
                    last = last->next;
                }
                last->parent = variable->parent;
                variable->parent->childs++;

                variable->child->prev = variable->prev;
                if(variable->prev != NULL)
                    variable->prev->next = variable->child;

                last->next = variable->next;
                if(variable->next != NULL)
                    variable->next->prev = last;

                if(variable->parent->child == variable){
                    variable->parent->child = variable->child;
                }

                variable->parent->childs--;

                variable->child = NULL;
                variable->next = NULL;
                variable->prev = NULL;
                variable->parent = NULL;
                doc_delete(variable, ".");

                rtn = last;
            }

            return rtn;
        break;

        default:                                                                    // squash it, place it next to parent
            // if(depth >= max_depth){
            //    
            // }
            
            return variable;
        break;
    }
}


/* ----------------------------------------- Private Macros --------------------------------- */

// macro for type agnostic
#define mem_alloc_cpy(data, type, size)   (_mem_alloc_cpy((void*)data, sizeof(type), size))


// Macro checking ----------------------------------

// check to see if is a string or binary data type
bool __check_string_bindata(doc *variable){
    switch(variable->type){
        case dt_string:
        case dt_const_string:
        case dt_bindata:
        case dt_const_bindata:
            return true;
        break;

        default:
            return false;
        break;
    }
}

// check if obj if a value
doc *__check_obj_is_value(doc *variable){    
    switch (variable->type)
    {
        case dt_array:
        case dt_obj:
        case dt_null:
            errno_doc_code_internal = errno_doc_trying_to_set_value_of_non_value_type_data_type;
            errno_msg_doc_internal = variable->name;
            return dummy_doc_internal_ptr;
        break;

        default:
            errno_doc_code_internal = errno_doc_ok;
            return variable;
        break;
    }
}

// get error code 
int __doc_get_error_code(void){
    return errno_doc_code_internal;
}

// check object for the for loop iterator macro
doc *__check_obj_ite_macro(doc *obj){
    if (obj == NULL){
        errno_doc_code_internal = errno_doc_null_passed_doc_ptr;
        return dummy_doc_internal_ptr;
    }
    else if(obj->type != dt_obj && obj->type != dt_array){
        errno_doc_code_internal = errno_doc_trying_to_get_data_from_non_object_or_non_array;
        return dummy_doc_internal_ptr;
    }
    else{
        return obj;
    }
}

/* ----------------------------------------- Functions -------------------------------------- */


// Error handling ----------------------------------

// get error msg
char *doc_get_error_msg(void){
    char *message = malloc(ERROR_MSG_LEN_DOC_HEADER + 1);
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
    switch (errno_doc_code_internal)
    {
        case errno_doc_ok:
        case errno_doc_overflow_quantity_members_or_name_is_too_big:                        // name can be too big to print
        case errno_doc_name_cointains_illegal_characters_or_missing_semi_colon_terminator:  // errno_msg_doc_internal can contain trashed ascii characters 
            snprintf(message, ERROR_MSG_LEN_DOC_HEADER - 1, "%s.", error_msg);
        break;


        default:
            snprintf(message, ERROR_MSG_LEN_DOC_HEADER - 1, "%s. Instance: \"%s\".", error_msg, errno_msg_doc_internal);
        break;
    }

    return message;
}


// Doc functions -----------------------------------

// create new object
doc *doc_new(char *name, doc_type_t type, ...){
    va_list args;
    va_start(args, type);

    doc *variable = parse_doc_syntax(name, type, &args);

    va_end(args);

    return variable;
}

// add new elements syntax to existing element
void doc_add(doc *object_or_array, char *name_to_add_to, char *name, doc_type_t type, ...){

    doc *variable = get_variable_ptr(object_or_array, name_to_add_to);              // get instance from name

    if(variable == NULL)                                                            // no instance of name found, return
        return;

    if(variable->type != dt_obj && variable->type != dt_array){                     // check if it is an array or object
        errno_doc_code_internal = errno_doc_trying_to_add_new_data_to_non_object_or_non_array;
        errno_msg_doc_internal = name_to_add_to;
        return;
    }

    va_list args;
    va_start(args, type);

    doc *new_variable = parse_doc_syntax(name, type, &args);

    va_end(args);
    
    if(new_variable == NULL)
        return;

    if(variable->type == dt_array && variable->child != NULL && variable->child->type != new_variable->type){     
        errno_doc_code_internal = errno_doc_value_not_same_type_as_array;           // check if array has a child already,
        errno_msg_doc_internal = name_to_add_to;                                    // if so, new element must be of the same type
        return;
    }

    variable->childs++;

    new_variable->parent = variable;
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

    errno_doc_code_internal = errno_doc_ok;
    return;
}

// delete element denoted by 'name'
void doc_delete(doc *variable, char *name){
    if(variable == NULL){
        errno_doc_code_internal = errno_doc_null_passed_doc_ptr;
        errno_msg_doc_internal = name;
        return;
    }

    doc *var = get_variable_ptr(variable, name);                                    // get pointer to instance
    
    if(var == NULL){
        errno_doc_code_internal = errno_doc_value_not_found;
        errno_msg_doc_internal = name;
        return;
    }

    switch(var->type){                                                     
        case dt_array:                                                              // in case of a entire OBJ OR ARRAY
        case dt_obj:
            for(doc* member = var->child; member != NULL; member = var->child){
                doc_delete(member, ".");
            }
        break;

        case dt_string:
            free(((doc_string*)var)->string);
        break;

        case dt_bindata:
            free(((doc_bindata*)var)->data);
        break;
    }

    if(var->parent != NULL){                                                        // if the variable is not a child, then is not part of an array, making next and prev pointer manipulation unnecessary 
        if(var->prev == NULL && var->next == NULL){                                 // last 1 elements 
            var->parent->child = NULL;
            var->parent->childs = 0;
        }
        else if(var->parent->child == var){                                         // first element 
            var->parent->child = var->next;
            var->next->parent = var->parent;
            var->next->prev = NULL;
            var->parent->childs--;
        }
        else if(var->next == NULL){                                                 // last element
            var->prev->next = var->next;
            var->parent->childs--;
        }
        else{                                                                       // any in middle element 
            var->prev->next = var->next;
            var->next->prev = var->prev;
            var->parent->childs--;
        }
    }

    free(var->name);
    free(var);

    errno_doc_code_internal = errno_doc_ok;
}

// get pointer to element
doc* doc_get_ptr(doc* variable, char *name){
    doc *var = get_variable_ptr(variable, name);

    if(var == NULL){
        errno_doc_code_internal = errno_doc_value_not_found;
        errno_msg_doc_internal  = name;
        return NULL; 
    }

    errno_doc_code_internal = errno_doc_ok;
    return var;
}

// get array and obj childs amount
doc_size_t doc_get_size(doc *variable, char *name){
    if(variable == NULL){
        errno_doc_code_internal = errno_doc_null_passed_doc_ptr;
        errno_msg_doc_internal = NULL;
        return 0;
    }
    
    doc *var = get_variable_ptr(variable, name);
    doc_size_t size;

    if(var == NULL){
        errno_doc_code_internal = errno_doc_value_not_found;
        errno_msg_doc_internal = name;
        return 0;
    }

    switch(var->type){
        case dt_obj:
        case dt_array:
            size = var->childs;
        break;
        
        case dt_string:
        case dt_const_string:
        case dt_bindata:
        case dt_const_bindata:
            size = ((doc_bindata*)var)->len;
        break;

        case dt_double:
            size = sizeof(double);
        break;
        case dt_float:
            size = sizeof(float);
        break;
        case dt_uint:
            size = sizeof(uint_t);
        break;
        case dt_uint64:
            size = sizeof(uint64_t);
        break;
        case dt_uint32:
            size = sizeof(uint32_t);
        break;
        case dt_uint16:
            size = sizeof(uint16_t);
        break;
        case dt_uint8:
            size = sizeof(uint8_t);
        break;
        case dt_int:
            size = sizeof(int);
        break;
        case dt_int64:
            size = sizeof(int64_t);
        break;
        case dt_int32:
            size = sizeof(int32_t);
        break;
        case dt_int16:
            size = sizeof(int16_t);
        break;
        case dt_int8:
            size = sizeof(int8_t);
        break;
        case dt_bool:
            size = sizeof(bool);
        break;

        case dt_null:
            size = 0;
        break;
    }

    errno_doc_code_internal = errno_doc_ok;
    return size;
}

// appends a already made variable to a object or array
void doc_append(doc *object_or_array, char *name, doc *variable){
    if(variable == NULL){
        errno_doc_code_internal = errno_doc_null_passed_doc_ptr;
        errno_msg_doc_internal  = variable->name;
        return;
    }

    doc *obj = get_variable_ptr(object_or_array, name);

    if(obj == NULL){
        errno_doc_code_internal = errno_doc_value_not_found;
        errno_msg_doc_internal = name;
        return;
    }

    if(obj->type != dt_obj && obj->type != dt_array){                     // check if it is an array or object
        errno_doc_code_internal = errno_doc_trying_to_add_new_data_to_non_object_or_non_array;
        errno_msg_doc_internal = variable->name;
        return;
    }

    variable->parent = obj;
    if(obj->child == NULL){
        obj->child = variable;
    }
    else{
        doc *cursor = obj->child;

        while(cursor->next != NULL)
            cursor = cursor->next;

        cursor->next = variable;
        variable->prev = cursor;
    }

    obj->childs++;

    errno_doc_code_internal = errno_doc_ok;
}

// allocate and copy a variable
doc *doc_copy(doc *variable, char *name){
    if(variable == NULL){
        errno_doc_code_internal = errno_doc_null_passed_doc_ptr;
        errno_msg_doc_internal  = variable->name;
        return NULL; 
    }

    variable = get_variable_ptr(variable, name);

    if(variable == NULL){
        errno_doc_code_internal = errno_doc_value_not_found;
        errno_msg_doc_internal = name;
        return NULL;
    }

    doc *copy = NULL;
    doc *cursor = NULL;
    doc *member = NULL;
    doc *last_member = NULL;

    switch (variable->type)
    {
        case dt_obj:
        case dt_array:
            copy = (doc*)mem_alloc_cpy(variable, doc, 1);

            cursor = copy->child;

            for(doc_size_t i = 0; i < copy->childs; i++){
                member = doc_copy(cursor, ".");
                
                if(i == 0){
                    copy->child = member;
                    member->prev = NULL;
                    member->next = NULL;
                    member->parent = copy;
                }
                else{
                    member->next = NULL;
                    member->prev = last_member;
                    last_member->next = member;
                }

                last_member = member;
                cursor = cursor->next;
            }

        break;
        
        case dt_null:
            copy = (doc*)mem_alloc_cpy(variable, doc, 1);
        break;

        case dt_bool:
            copy = (doc*)mem_alloc_cpy(variable, doc_bool, 1);
        break;

        case dt_double:
            copy = (doc*)mem_alloc_cpy(variable, doc_double, 1);
        break;
        
        case dt_float:
            copy = (doc*)mem_alloc_cpy(variable, doc_float, 1);
        break;

        case dt_uint:
            copy = (doc*)mem_alloc_cpy(variable, doc_uint_t, 1);
        break;

        case dt_uint64:
            copy = (doc*)mem_alloc_cpy(variable, doc_uint64_t, 1);
        break;

        case dt_uint32:
            copy = (doc*)mem_alloc_cpy(variable, doc_uint32_t, 1);
        break;

        case dt_uint16:
            copy = (doc*)mem_alloc_cpy(variable, doc_uint16_t, 1);
        break;

        case dt_uint8:
            copy = (doc*)mem_alloc_cpy(variable, doc_uint8_t, 1);
        break;

        case dt_int:
            copy = (doc*)mem_alloc_cpy(variable, doc_int, 1);
        break;

        case dt_int64:
            copy = (doc*)mem_alloc_cpy(variable, doc_int64_t, 1);
        break;

        case dt_int32:
            copy = (doc*)mem_alloc_cpy(variable, doc_int32_t, 1);
        break;

        case dt_int16:
            copy = (doc*)mem_alloc_cpy(variable, doc_int16_t, 1);
        break;

        case dt_int8:
            copy = (doc*)mem_alloc_cpy(variable, doc_int8_t, 1);
        break;

        case dt_string:
            copy = (doc*)mem_alloc_cpy(variable, doc_string, 1);
            ((doc_string*)copy)->string = (char*)mem_alloc_cpy(((doc_string*)variable)->string, char, ((doc_string*)variable)->len);
        break;

        case dt_const_string:
            copy = (doc*)mem_alloc_cpy(variable, doc_string, 1);
        break;

        case dt_bindata:
            copy = (doc*)mem_alloc_cpy(variable, doc_bindata, 1);
            ((doc_bindata*)copy)->data = (uint8_t*)mem_alloc_cpy(((doc_bindata*)variable)->data, uint8_t, ((doc_bindata*)variable)->len);
        break;

        case dt_const_bindata:
            copy = (doc*)mem_alloc_cpy(variable, doc_bindata, 1);
        break;
    }

    copy->next = NULL;
    copy->prev = NULL;
    copy->parent = NULL;
    copy->name = mem_alloc_cpy(variable->name, char, strlen(variable->name) + 1);
    if(copy->type != dt_array && copy->type != dt_obj){
        copy->child = NULL;
        copy->childs = 0;
    }

    errno_doc_code_internal = errno_doc_ok;
    return copy;
}

// rename variable
void doc_rename(doc *variable, char *name, char *new_name){
    if(variable == NULL){
        errno_doc_code_internal = errno_doc_null_passed_doc_ptr;
        errno_msg_doc_internal  = variable->name;
        return;
    }

    variable = get_variable_ptr(variable, name);

    if(variable == NULL){
        errno_doc_code_internal = errno_doc_value_not_found;
        errno_msg_doc_internal = name;
        return;
    }
    
    free(variable->name);
    variable->name = (char*)mem_alloc_cpy(new_name, char, strlen(new_name) + 1);
}

// squash a variable and its childs by a maximun nesting depth
void doc_squash(doc *variable, char *name, doc_size_t max_depth){
    if(variable == NULL){
        errno_doc_code_internal = errno_doc_null_passed_doc_ptr;
        errno_msg_doc_internal  = variable->name;
        return;
    }

    variable = get_variable_ptr(variable, name);

    if(variable == NULL){
        errno_doc_code_internal = errno_doc_value_not_found;
        errno_msg_doc_internal = name;
        return;
    }
    
    switch(variable->type){

        case dt_obj:
        case dt_array:
            if(max_depth == 0){
                errno_doc_code_internal = errno_doc_trying_to_squash_a_doc_structure_to_0__This_is_not_possible_becaue_it_needs_at_least_one_object_to_hold_data;
                errno_msg_doc_internal = name;
                return;
            }
            else{
                squash(variable, max_depth, 0);
            }
        break;

        default:
            return;
        break;
    }
}

// create a new automatic doc variable from a string
doc *doc_from_string(char *name, char *string){
    if(name == NULL){
        errno_doc_code_internal = errno_doc_null_passed_parameter;
        errno_msg_doc_internal = "parameter: name";
        return NULL;
    }
    
    if(string == NULL){
        errno_doc_code_internal = errno_doc_null_passed_parameter;
        errno_msg_doc_internal = "parameter: string";
        return NULL;
    }
    
    return create_doc_from_string(name, string);
}

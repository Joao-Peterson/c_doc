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
    errno_doc_ok                                                                    =  0,
    errno_doc_not_a_type                                                            = -1,
    errno_doc_overflow_quantity_members_or_name_is_too_big                          = -2,
    errno_doc_value_not_same_type_as_array                                          = -3,
    errno_doc_duplicate_names                                                       = -4,
    errno_doc_null_passed_obj                                                       = -5,
    errno_doc_obj_not_found                                                         = -6,
    errno_doc_name_cointains_illegal_characters_or_missing_semi_colon_terminator    = -7
}errno_doc_code_t;

/* ----------------------------------------- Private Struct's --------------------------------- */

// error struct for ease error handling 
typedef struct{
    errno_doc_code_t code;
    char *msg;
}errno_doc_t;

/* ----------------------------------------- Private Globals -------------------------------- */

// dummy instance to receive macros operations, to not generate segfault
doc_uint64_t dummy_doc_internal;
doc *dummy_doc_internal_ptr = (doc*)&dummy_doc_internal;

// ilegal ascii characters on names
const char *illegal_chars_doc_name = "\a\b\t\n\v\f\r\"\'()*+,-.\\";
// const char *illegal_chars_doc_name = "\1\2\3\4\5\6\a\b\t\n\v\f\r\14\15\16\17\18\19\20\21\22\23\24\25\26\27\28\29\30\31\32\"\37\'()*+,-.\\\127\128\129\130\131\132\133\134\135\136\137\138\139\140\141\142\143\144\145\146\147\148\149\150\151\152\153\154\155\156\157\158\159\160\161\162\163\164\165\166\167\168\169\170\171\172\173\174\175\176\177\178\179\180\181\182\183\184\185\186\187\188\189\190\191\192\193\194\195\196\197\198\199\200\201\202\203\204\205\206\207\208\209\210\211\212\213\214\215\216\217\218\219\220\221\222\223\224\225\226\227\228\229\230\231\232\233\234\235\236\237\238\239\240\241\242\243\244\245\246\247\248\249\250\251\252\253\254";

// internal error vars, the char * one holds information about the name of instance to be acted on
errno_doc_code_t errno_doc_code_internal = 0;
char *errno_msg_doc_internal = NULL;

// array to get the value and name of defined errors 
const errno_doc_t errno_doc_msg_code_array[] = {
    ERR_TO_STRUCT(errno_doc_ok),
    ERR_TO_STRUCT(errno_doc_not_a_type),
    ERR_TO_STRUCT(errno_doc_overflow_quantity_members_or_name_is_too_big),
    ERR_TO_STRUCT(errno_doc_value_not_same_type_as_array),
    ERR_TO_STRUCT(errno_doc_duplicate_names),
    ERR_TO_STRUCT(errno_doc_null_passed_obj),
    ERR_TO_STRUCT(errno_doc_obj_not_found),
    ERR_TO_STRUCT(errno_doc_name_cointains_illegal_characters_or_missing_semi_colon_terminator)
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

    char *char_veri = strpbrk(name,illegal_chars_doc_name);
    if( char_veri != NULL){
        errno_doc_code_internal = errno_doc_name_cointains_illegal_characters_or_missing_semi_colon_terminator;
        errno_msg_doc_internal = name;
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

                if(i > MAX_OBJ_MEMBER_QTY || strlen(member_name) > DOC_NAME_MAX_LEN){
                    errno_doc_code_internal = errno_doc_overflow_quantity_members_or_name_is_too_big;
                    errno_msg_doc_internal = variable->name;
                    return NULL;
                }

                if(member_name[0] == ';')                                           // quit when terminator char found
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
    
    if(object_or_array == NULL){
        errno_doc_code_internal = errno_doc_null_passed_obj;
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

    if(name_cpy[0] == '.'){ name_cpy++; }                                           // jump over optional '.' at the beginning of path
    char *var_name = name_cpy;
    char *var_name_next = strpbrk(var_name, ".");

    if(var_name_next != NULL){                                                      // on the last one
        var_name_next[0] = '\0';                                                    // make the first name acessible directly
        var_name_next++;                                                            // points to other name
    }

    do{                                                                             // search names
        if(!strcmp(cursor->name,var_name)){
            if(var_name_next == NULL){
                free(name_cpy_base);
                return cursor;                                                      // if there are no elements left, return this one
            }
            else{
                doc *return_ptr = get_variable_ptr(cursor, var_name_next);          // else, call the function again on the left elements
                free(name_cpy_base);
                return return_ptr;
            }
        }

        cursor = cursor->next;
    }while(cursor != NULL);

    free(name_cpy_base);
    return NULL;
}

/* ----------------------------------------- Functions -------------------------------------- */

// check if obj is null, used on macros
doc *__check_obj(doc *obj){
    if(obj == NULL){
        errno_doc_code_internal = errno_doc_null_passed_obj;
        errno_msg_doc_internal = obj->name;
        return dummy_doc_internal_ptr;
    }
    else
        return obj;
}

// get error code 
int __doc_get_error_code(void){
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

doc *doc_new(char *name, doc_type_t type, ...){
    va_list args;
    va_start(args, type);

    doc *variable = parse_doc_syntax(name, type, &args);

    va_end(args);

    return variable;
}

void doc_add(doc *object_or_array, char *name_to_add_to, char *name, doc_type_t type, ...){

    doc *variable = get_variable_ptr(object_or_array, name_to_add_to);              // get instance from name

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

    errno_doc_code_internal = errno_doc_ok;
    return;
}

void doc_delete(doc *object_or_array, char *name){
    
    doc *variable = get_variable_ptr(object_or_array, name);                    // get pointer to instance
    if(variable == NULL)
        return;

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

    errno_doc_code_internal = errno_doc_ok;
}

doc* doc_get(doc* object_or_array, char *name){
    // check to see if doc* is obj/array
    if(object_or_array->type != dt_obj && object_or_array->type != dt_array){
        errno_doc_code_internal = errno_doc_null_passed_obj;
        errno_msg_doc_internal = "null";
        return NULL;
    }

    doc *variable = get_variable_ptr(object_or_array, name);

    if(variable == NULL){
        errno_doc_code_internal = errno_doc_obj_not_found;
        errno_msg_doc_internal  = name;
        return NULL; 
    }

    errno_doc_code_internal = errno_doc_ok;
    return variable;
}

void doc_set_string(doc *obj, char *name, char *new_string, size_t new_len){
    ((doc_string*)get_variable_ptr(obj,name))->string = new_string;         
    ((doc_string*)get_variable_ptr(obj,name))->len = new_len; 
    errno_doc_code_internal = errno_doc_ok;
}

void doc_set_bindata(doc *obj, char *name, char *new_data, size_t new_len){
    ((doc_bindata*)get_variable_ptr(obj,name))->data = new_data;            
    ((doc_bindata*)get_variable_ptr(obj,name))->len = new_len;
    errno_doc_code_internal = errno_doc_ok;
}


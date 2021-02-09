#include "doc.h"
#include "doc_json.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define VALUE_TOKEN_SEQ             ("\"-0123456789.{[tfn")
#define VALUE_TOKEN_SEQ_W_SQR_BRK   ("\"-0123456789.{[tfn]")
#define TERMINATORS                 (",}]")
#define WHITE_SPACE                 (" \t\n\r\v\f")
#define WHITE_SPACE_W_COMMA         (" \t\n\r\v\f,")

// data type for rational javascript numbers
// data type for rational javascript numbers
#define RATIONAL_TYPE (dt_double)                                                   // to be put on 'type' member of 'doc'
#define strto_rational(const_char_ptr_string, const_char_ptr_ptr_endptr) strtod(const_char_ptr_string, const_char_ptr_ptr_endptr)               // to convert to number
typedef doc_double      rational_number_t;                                          // to allocate the correct 'doc_*' type
                                                                                    
// data type for rational javascript numbers
#define INTEGER_TYPE (dt_int32)                                                     // to be put on 'type' member of 'doc'
#define strto_integer(const_char_ptr_string, const_char_ptr_ptr_endptr) atoi(const_char_ptr_string)                                             // to convert to number
typedef doc_int32_t     integer_number_t;                                           // to allocate the correct 'doc_*' type


char *parse_string(char **string){
    (*string) = strpbrk((*string), "\"") + 1;                                           // locate string beggining

    char *special = (*string) - 2;                                                  // +2 because of -=2 inside do while
    char *last_special = (*string);
    
    do{                                                                             // jump over all \" inside the string
        special += 2;
        last_special = special;
        special = strstr(special, "\\\"");
    }while(special != NULL);

    char *end_ptr = strpbrk(last_special, "\"");                                    // get the termianator " character
    size_t span = end_ptr - (*string);

    char *read_string = malloc(span + 1);

    strncpy(read_string, (*string), span);

    read_string[span] = '\0';

    (*string) = end_ptr + 1;                                                       // make passed pointer point to after the string read

    return read_string;
}

doc *parse_value(char **string){    
    doc *variable = NULL;
    
    // char *value_begin = strpbrk((*string), ":");
    
    char *value_begin = strpbrk((*string), VALUE_TOKEN_SEQ);

    childs_amount_t i = 0;
    char *control = NULL;
    char *value_end = NULL;
    doc *last_member = NULL;

    switch(*value_begin){
        case '{':                                                                   // object
            variable = calloc(1, sizeof(doc));
            variable->type = dt_obj;
            
            i = 0;
            control = strpbrk(value_begin, "\"}");

            while((*control) != '}'){                                               // recursevely parse members
                char *member_name = parse_string(&control);
                doc *member = parse_value(&control);
                member->name = member_name;
                member->parent = variable;

                if(i == 0){                                                         // first member
                    variable->child = member;
                }
                else{                                                               // other members
                    last_member->next = member;
                    member->prev = last_member;
                }

                last_member = member;

                i++;
                control = strpbrk(control, "\"}");
            }
            
            variable->childs = i;
            
            (*string) = control + 1;
        break;

        case '[':                                                                   // array
            variable = calloc(1, sizeof(doc));
            variable->type = dt_array;
            
            i = 0;

            value_begin ++;
            control = strpbrk(value_begin, VALUE_TOKEN_SEQ_W_SQR_BRK);

            while((*control) != ']'){                                               // recursevely parse members
                doc *member = parse_value(&control);

                member->name = malloc(sizeof(char)*1);
                strncpy(member->name, "", 1);

                member->parent = variable;

                if(i == 0){                                                         // first member
                    variable->child = member;
                }
                else{                                                               // other members
                    last_member->next = member;
                    member->prev = last_member;
                }

                last_member = member;

                i++;
                control = strpbrk(control, VALUE_TOKEN_SEQ_W_SQR_BRK);
            }

            variable->childs = i;
            
            (*string) = control + 1;
        break;

        case '\"':                                                                  // string
            variable = calloc(1, sizeof(doc_string));
            doc_string *variable_string = (doc_string *)variable;
            variable_string->header.type = dt_string;

            variable_string->string = parse_string(&value_begin);

            (*string) = value_begin;
        break;

        case 't':                                                                   // true
            variable = calloc(1, sizeof(doc_bool));
            doc_bool *variable_bool_true = (doc_bool *)variable;
            variable_bool_true->value = true;
            variable_bool_true->header.type = dt_bool;

            (*string) = strpbrk((*string), TERMINATORS);
        break;

        case 'f':                                                                   // false
            variable = calloc(1, sizeof(doc_bool));
            doc_bool *variable_bool_false = (doc_bool *)variable;
            variable_bool_false->value = false;
            variable_bool_false->header.type = dt_bool;

            (*string) = strpbrk((*string), TERMINATORS);
        break;

        case 'n':                                                                   // null
            variable = calloc(1, sizeof(*variable));
            variable->type = dt_null;

            (*string) = strpbrk((*string), TERMINATORS);
        break;

        default:                                                                    // number
            value_end = strpbrk(value_begin, TERMINATORS);
            size_t value_len = value_end - value_begin;
            
            char *value_cpy = malloc(value_len + 1);
            strncpy(value_cpy, value_begin, value_len);
            value_cpy[value_len] = '\0';

            if(strpbrk(value_cpy, ".eE") != NULL){                                // if value contains decimal or exponencial terms
                variable = calloc(1, sizeof(rational_number_t));
                rational_number_t *variable_number_rational = (rational_number_t *)variable;

                variable_number_rational->value = strto_rational(value_cpy, NULL);
                variable_number_rational->header.type = RATIONAL_TYPE;

            }else{                                                                  // value is integer type
                variable = calloc(1, sizeof(integer_number_t));
                integer_number_t *variable_number_integer = (integer_number_t *)variable;

                variable_number_integer->value = strto_integer(value_cpy, NULL);
                variable_number_integer->header.type = INTEGER_TYPE;

            }

            free(value_cpy);

            (*string) = value_end;
        break;
    }

    return variable;
}

doc *doc_parse_json(char *file_stream){
    char *cursor = NULL;
    doc *null_return = calloc(1, sizeof(*null_return));
    null_return->type = dt_null;

    cursor = strpbrk(file_stream, "{");                                 
    char *end_check = strpbrk(cursor, "\"}");

    if(*end_check == '}')
        return null_return;

    doc *json = parse_value(&cursor);

    const char *name = "json";
    size_t name_len = strlen(name);
    json->name = malloc(sizeof(*name)*name_len + 1);
    strncpy(json->name, name, name_len + 1);
    
    return json; 
} 

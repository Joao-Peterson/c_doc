#include "doc_json.h"
#include "parse_utils.h"

/* ----------------------------------------- Definitions ------------------------------------ */

#define VALUE_TOKEN_SEQ             ("\"-0123456789.{[tfn")                         // to find any value type, string, array, obj, number, bool or null
#define VALUE_TOKEN_SEQ_W_SQR_BRK   ("\"-0123456789.{[tfn]")                        // same as above but with closing sqr brk, for anonymous members in arrays
#define TERMINATORS                 (",}]")                                         // to check end of member, obj or array
#define WHITE_SPACE                 (" \t\n\r\v\f")                                 // white space chars                                      // to allocate the correct 'doc_*' type

/* ----------------------------------------- Private Functions ------------------------------ */

// parse a string from '"' to the end '"', and cat it to the *string
static char *parse_string(char **string){
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

// parse a value after ':', may it be any json type and cat it to the *string, recursive
static doc *parse_value(char **string){    
    doc *variable = NULL;
    
    // char *value_begin = strpbrk((*string), ":");
    
    char *value_begin = strpbrk((*string), VALUE_TOKEN_SEQ);

    doc_size_t i = 0;
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
            variable_string->len    = strlen(variable_string->string);

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
                variable = calloc(1, sizeof(decimal_doc_type_parse_utils));
                decimal_doc_type_parse_utils *variable_number_rational = (decimal_doc_type_parse_utils *)variable;

                variable_number_rational->value = strto_rational_parse_utils(value_cpy, NULL);
                variable_number_rational->header.type = decimal_dt_type_parse_utils;

            }else{                                                                  // value is integer type
                variable = calloc(1, sizeof(integer_doc_type_parse_utils));
                integer_doc_type_parse_utils *variable_number_integer = (integer_doc_type_parse_utils *)variable;

                variable_number_integer->value = strto_integer_parse_utils(value_cpy, NULL);
                variable_number_integer->header.type = integer_dt_type_parse_utils;

            }

            free(value_cpy);

            (*string) = value_end;
        break;
    }

    return variable;
}

// recursive function call to create the json
static void stringify(doc *variable, char **base_address, size_t *length){

    char *value = NULL;
    char *buffer = NULL;
    doc *member = NULL;
    size_t value_len;
    bool first_call = false;

    if(*base_address == NULL){
        *base_address = calloc(1, sizeof(**base_address));
        *length = 1;
        first_call = true;
    }
    
    switch(variable->type){

        case dt_obj:
        case dt_array:

            if(*(variable->name) == '\0' || first_call){                            // anonymous obj
                (*length) += 1;
                (*base_address) = realloc(*base_address, *length);
                
                if(variable->type == dt_obj){ strcat(*base_address, "{"); }else{ strcat(*base_address, "["); }
            }                         
            else{
                value_len = strlen(variable->name) + 5;                             // "\"name\":{" has (strlen + 5) chars
                value = calloc(1, sizeof(*value) * value_len);

                if(variable->type == dt_obj){ 
                    snprintf(value, value_len, "\"%s\":{", variable->name); 
                }
                else{ 
                    snprintf(value, value_len, "\"%s\":[", variable->name); 
                }

                (*length) += value_len;
                (*base_address) = realloc(*base_address, *length);
                strcat(*base_address, value);
            }                  

            member = variable->child;

            for(doc_size_t i = 0; i < variable->childs; i++){
                if( i != 0 )
                    strcat(*base_address, ",");                                     // cat comma before every member, except on the first
                
                stringify(member, base_address, length);

                (*length) += 1;
                (*base_address) = realloc(*base_address, *length);
                member = member->next;
            }

            (*length) += 1;
            (*base_address) = realloc(*base_address, *length);
            if(variable->type == dt_obj){ strcat(*base_address, "}"); }else{ strcat(*base_address, "]"); }
            
        break;

        case dt_double:
            value_len = strlen(variable->name) + 4;
            value = calloc(value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, sizeof(*value));     

            if(variable->name[0] != '\0')
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "\"%s\":%.*G", variable->name, FLOAT_DECIMAL_PLACES_PARSE_UTILS, ((doc_double *)(variable))->value);
            else
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "%.*G", FLOAT_DECIMAL_PLACES_PARSE_UTILS, ((doc_double *)(variable))->value);

            *length += strlen(value);
            *base_address = realloc(*base_address, *length);
            strcat(*base_address, value);
        break;
            
        case dt_float:
            value_len = strlen(variable->name) + 4;
            value = calloc(value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, sizeof(*value));     
            
            if(variable->name[0] != '\0')
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "\"%s\":%.*G", variable->name, FLOAT_DECIMAL_PLACES_PARSE_UTILS, ((doc_float *)(variable))->value);
            else
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "%.*G", FLOAT_DECIMAL_PLACES_PARSE_UTILS, ((doc_float *)(variable))->value);

            *length += strlen(value);
            *base_address = realloc(*base_address, *length);
            strcat(*base_address, value);
        break;

        case dt_int:
            value_len = strlen(variable->name) + 4;
            value = calloc(UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + value_len, sizeof(*value));     

            if(variable->name[0] != '\0')
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "\"%s\":%i", variable->name, ((doc_int *)(variable))->value);
            else
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "%i", ((doc_int *)(variable))->value);

            *length += strlen(value);
            *base_address = realloc(*base_address, *length);
            strcat(*base_address, value);
        break;
 
        case dt_int8:
            value_len = strlen(variable->name) + 4;
            value = calloc(UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + value_len, sizeof(*value));     

            if(variable->name[0] != '\0')
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "\"%s\":%i", variable->name, ((doc_int8_t *)(variable))->value);
            else
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "%i", ((doc_int8_t *)(variable))->value);

            *length += strlen(value);
            *base_address = realloc(*base_address, *length);
            strcat(*base_address, value);
        break;
 
        case dt_int16:
            value_len = strlen(variable->name) + 4;
            value = calloc(UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + value_len, sizeof(*value));     

            if(variable->name[0] != '\0')
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "\"%s\":%i", variable->name, ((doc_int16_t *)(variable))->value);
            else
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "%i", ((doc_int16_t *)(variable))->value);

            *length += strlen(value);
            *base_address = realloc(*base_address, *length);
            strcat(*base_address, value);
        break;
 
        case dt_int32:
            value_len = strlen(variable->name) + 4;
            value = calloc(UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + value_len, sizeof(*value));     
            
            if(variable->name[0] != '\0')
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "\"%s\":%i", variable->name, ((doc_int32_t *)(variable))->value);
            else
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "%i", ((doc_int32_t *)(variable))->value);

            *length += strlen(value);
            *base_address = realloc(*base_address, *length);
            strcat(*base_address, value);
        break;
 
        case dt_int64:        
            value_len = strlen(variable->name) + 4;
            value = calloc(UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + value_len, sizeof(*value));     

            if(variable->name[0] != '\0')
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "\"%s\":%i", variable->name, ((doc_int64_t *)(variable))->value);
            else
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "%i", ((doc_int64_t *)(variable))->value);

            *length += strlen(value);
            *base_address = realloc(*base_address, *length);
            strcat(*base_address, value);
        break;
 
        case dt_uint:
            value_len = strlen(variable->name) + 4;
            value = calloc(UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + value_len, sizeof(*value));     

            if(variable->name[0] != '\0')
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "\"%s\":%u", variable->name, ((doc_uint_t *)(variable))->value);
            else
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "%u", ((doc_uint_t *)(variable))->value);

            *length += strlen(value);
            *base_address = realloc(*base_address, *length);
            strcat(*base_address, value);
        break;
 
        case dt_uint8:
            value_len = strlen(variable->name) + 4;
            value = calloc(UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + value_len, sizeof(*value));     

            if(variable->name[0] != '\0')
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "\"%s\":%u", variable->name, ((doc_uint8_t *)(variable))->value);
            else
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "%u", ((doc_uint8_t *)(variable))->value);

            *length += strlen(value);
            *base_address = realloc(*base_address, *length);
            strcat(*base_address, value);
        break;
 
        case dt_uint16:
            value_len = strlen(variable->name) + 4;
            value = calloc(UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + value_len, sizeof(*value));     

            if(variable->name[0] != '\0')
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "\"%s\":%u", variable->name, ((doc_uint16_t *)(variable))->value);
            else
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "%u", ((doc_uint16_t *)(variable))->value);

            *length += strlen(value);
            *base_address = realloc(*base_address, *length);
            strcat(*base_address, value);
        break;
 
        case dt_uint32:
            value_len = strlen(variable->name) + 4;
            value = calloc(UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + value_len, sizeof(*value));     

            if(variable->name[0] != '\0')
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "\"%s\":%u", variable->name, ((doc_uint32_t *)(variable))->value);
            else
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "%u", ((doc_uint32_t *)(variable))->value);

            *length += strlen(value);
            *base_address = realloc(*base_address, *length);
            strcat(*base_address, value);
        break;
 
        case dt_uint64:
            value_len = strlen(variable->name) + 4;
            value = calloc(UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + value_len, sizeof(*value));     

            if(variable->name[0] != '\0')
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "\"%s\":%u", variable->name, ((doc_uint64_t *)(variable))->value);
            else
                snprintf(value, value_len + FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "%u", ((doc_uint64_t *)(variable))->value);

            *length += strlen(value);
            *base_address = realloc(*base_address, *length);
            strcat(*base_address, value);
        break;

        case dt_bool:
            value_len = strlen(variable->name) + 6 + 4;
            value = calloc(value_len, sizeof(*value));     

            if(variable->name[0] != '\0')
                snprintf(value, value_len, "\"%s\":%s", variable->name, (((doc_bool *)(variable))->value) ? "true" : "false" );
            else
                snprintf(value, value_len, "%s", (((doc_bool *)(variable))->value) ? "true" : "false" );

            *length += strlen(value);            
            *base_address = realloc(*base_address, *length);
            strcat(*base_address, value);
        break;

        case dt_null:
            value_len = strlen(variable->name) + 5 + 4;
            value = calloc(value_len, sizeof(*value));     

            if(variable->name[0] != '\0')
                snprintf(value, value_len, "\"%s\":%s", variable->name, "null");
            else
                snprintf(value, value_len, "%s", "null");

            *length += strlen(value);
            *base_address = realloc(*base_address, *length);
            strcat(*base_address, value);
        break;

        case dt_string:
        case dt_const_string:
            value_len = strlen(variable->name) + ((doc_string *)variable)->len + 6;
            value = calloc(1, value_len);

            if(variable->name[0] != '\0')
                snprintf(value, value_len, "\"%s\":\"%s\"", variable->name, ((doc_string *)variable)->string);
            else
                snprintf(value, value_len, "\"%s\"", ((doc_string *)variable)->string);

            *length += strlen(value);
            *base_address = realloc(*base_address, *length);
            strcat(*base_address, value);
        break;

        case dt_bindata:
        case dt_const_bindata:
            buffer = base64_encode(((doc_bindata *)variable)->data, ((doc_bindata *)variable)->len);            

            value_len = strlen(variable->name) + strlen(buffer) + 6;
            value = calloc(1, value_len);

            if(variable->name[0] != '\0')
                snprintf(value, value_len, "\"%s\":\"%s\"", variable->name, buffer);
            else
                snprintf(value, value_len, "\"%s\"", buffer);

            free(buffer);

            *length += strlen(value);
            *base_address = realloc(*base_address, *length);
            strcat(*base_address, value);
        break;
    }

    if(value != NULL)
        free(value);
}

/* ----------------------------------------- Functions -------------------------------------- */

// opens and parse a json file to a doc structure
doc *doc_json_open(char *filename){
    if(filename == NULL){
        return NULL;
    }

    char *stream = fstream(filename);

    doc *json = doc_json_parse(stream);

    free(stream);

    return json;
}

// save doc json to file
void doc_json_save(doc *json_doc, char *filename){
    char *json = doc_json_stringify(json_doc);

    if(json == NULL) return;

    FILE *out = fopen(filename, "w+");
    fprintf(out, json);
    fclose(out);
    free(json);
}

// parse json
doc *doc_json_parse(char *file_stream){
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

// make a json out of doc
char *doc_json_stringify(doc *json_doc){
    char *json_stream = NULL;
    size_t json_stream_len = 0;

    if(json_doc->type != dt_obj)
        return NULL;

    stringify(json_doc, &json_stream, &json_stream_len);

    return json_stream;
}

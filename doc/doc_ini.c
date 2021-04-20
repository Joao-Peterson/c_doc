#include "doc_ini.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../base64/base64.h"

/* ----------------------------------------- Private Definitions ---------------------------- */

#define UINT64_MAX_DECIMAL_CHARS_INI    (20)
#define FLOAT_MAX_DECIMAL_CHARS_INI     (27)

/* ----------------------------------------- Private Globals -------------------------------- */

static const char *NUMBER_INTEGER_ALPHABET = "0123456789-+";
static const char *NUMBER_DECIMAL_ALPHABET = "0123456789.,eE-+";

/* ----------------------------------------- Numeric values config -------------------------- */

typedef int64_t             integer_ini_t;                                          // type for integer values
#define integer_ini_doc_ini dt_int64                                                // doc type for integer values

#define strto_integer_ini(const_char_ptr_string, const_char_ptr_ptr_endptr) \
    atoi(const_char_ptr_string)                                                     // function to convert from string to integer


#define FLOAT_DECIMAL_PLACES_INI    (5)                                             // decimal places after dot on decimals
#define decimal_print_format    "%#.*G"                                              // to print decimal values    
typedef double              decimal_ini_t;                                          // type for decimal values
#define decimal_ini_doc_ini dt_double                                               // doc type for decimal values

#define strto_rational_ini(const_char_ptr_string, const_char_ptr_ptr_endptr) \
    strtod(const_char_ptr_string, const_char_ptr_ptr_endptr)                        // function to convert from string to decimal

/* ----------------------------------------- Private Functions ------------------------------ */

// run whitespace except for the '\n' line break
static void run_space(char **stream){
    while( **stream != '\n' && ((**stream >= '\b' && **stream <= '\r') || **stream == 32) )
        (*stream)++;
}

// run all whitespace chars
static void run_whitespace(char **stream){
    while( ((**stream >= '\b' && **stream <= '\r') || **stream == 32) )
        (*stream)++;
}

// run whitespace backwards
static void run_whitespace_back(char **stream){
    while( ((**stream >= '\b' && **stream <= '\r') || **stream == 32) )
        (*stream)--;
}



// check if a string if formed by just members of a defined alphabet 
static bool str_alphabet(char *string, char *alphabet){
    bool flag = true;
    size_t len = strlen(string);
    size_t counter = strlen(alphabet);

    if(*string == '\0') return false;                                               // empty string

    for(size_t i = 0; i < len; i++){
        bool check = false;

        for(size_t j = 0; j < counter; j++){
            if(string[i] == alphabet[j]){
                check = true; 
                break;
            }
        }

        if(!check){
            flag = false;
            break;
        }
    }

    return flag;
}

// check how many times a char appears in a string
static size_t strnchr(char *string, char chr){
    size_t count = 0;

    for(size_t i = 0; i < strlen(string); i++){
        if(string[i] == chr)
            count++;
    }

    return count;
}

// check if string represents a number
static bool str_is_number(char *string){
    run_whitespace(&string);

    if( (*string < '0' && *string > '9') && *string != '+' && *string != '-')
        return false;

    if( strnchr(string, '-') > 1 || 
        strnchr(string, '+') > 1 || 
        strnchr(string, '.') > 1 ||
        strnchr(string, ',') > 1 ||
        strnchr(string, 'e') > 1 ||
        strnchr(string, 'E') > 1
    )
        return false;

    return true;
}

// check the value of a string representation of the value
static doc_type_t check_value_type(char *value){
    if(!strcmp(value, "true") || !strcmp(value, "false")){                          // boolean
        return dt_bool;
    }
    else if(str_alphabet(value, (char *)NUMBER_INTEGER_ALPHABET)){                  // integer                                  
        if(str_is_number(value))
            return integer_ini_doc_ini;
        else
            return dt_string;
    }
    else if(str_alphabet(value, (char *)NUMBER_DECIMAL_ALPHABET)){                  // decimal                                  
        if(str_is_number(value))
            return decimal_ini_doc_ini;
        else
            return dt_string;
    }
    else{                                                                           // string
        return dt_string;
    }
}


// create a doc from a string with an appropriate value type
static doc *doc_from_string(char *name, char *value_string){
    doc_type_t type = check_value_type(value_string);
    doc *variable;
    bool boolean_buf;

    switch(type){
        case decimal_ini_doc_ini:
            variable = doc_new(name, decimal_ini_doc_ini, strto_rational_ini(value_string, NULL));
        break;

        case integer_ini_doc_ini:
            variable = doc_new(name, integer_ini_doc_ini, strto_integer_ini(value_string, NULL));
        break;

        case dt_bool:
            if(!strcmp(value_string, "true"))
                boolean_buf = true;
            else
                boolean_buf = false;

            variable = doc_new(name, dt_bool, boolean_buf);
        break;
        
        case dt_string:
        default:
            variable = doc_new(name, dt_string, value_string, strlen(value_string) + 1);
        break;  
    }

    return variable;
}

// crop and join a space inside a string
static void strcrop(char *string, char *start, char *end){
    *start = '\0';
    strcat(string, end);
}

// clean a string with line breaking sequence
static char *strbreak_clear(char *string){
    char *string_copy = (char*)calloc(strlen(string) + 1, sizeof(char));
    memcpy(string_copy, string, strlen(string) + 1);
    
    for(char *cursor = strpbrk(string_copy, "\\"); cursor != NULL; cursor = strpbrk(string_copy, "\\")){
        char *start = cursor;

        cursor++;
        cursor = strpbrk(cursor, "\n");
        cursor++;

        char *end = cursor;

        strcrop(string_copy, start, end);
    }

    return string_copy;
}



// parse a section tag
static doc *parse_section(char **stream){
    if(*stream == NULL) return NULL;

    (*stream)++;
    char *name = *stream;

    *stream = strpbrk(*stream, "]");
    **stream = '\0';
    (*stream)++;

    doc *section = doc_new(name, dt_obj, ";");

    return section;
}

// parse a variable
static doc *parse_variable(char **stream){
    if(*stream == NULL) return NULL;
    
    doc *variable;
    char *lstring;

    run_whitespace(stream);
    char *name = *stream;

    if(**stream == '{'){                                                            // anonymous variable
        (*stream)++;

        lstring = *stream;

        *stream = strpbrk(*stream, "}");
        
        char hold = **stream;
        **stream = '\0';

        variable = doc_from_string("", lstring);

        **stream = hold;
        *stream = strpbrk(*stream, "}");
        (*stream)++;
    }
    else{                                                                           // normal variable
        *stream = strpbrk(*stream, "=\n");

        if(**stream == '='){                                                        // defined value
            size_t len;

            char *name_end = *stream;
            name_end--;
            run_whitespace_back(&name_end);
            name_end++;
            *name_end = '\0';

            (*stream)++;
            run_space(stream);

            if(**stream == '\"'){                                                   // string literal
                (*stream)++;
                lstring = *stream;

                // run trought the stream verifing the '\"' token and the ending '"' token 
                for(*stream = strpbrk(*stream, "\""); *(*stream - 1) == '\\'; *stream = strpbrk(*stream, "\"")); 

                **stream = '\0';
                len = *stream - lstring + 1;
                (*stream)++;
                *stream = strpbrk(*stream, "\n");
                (*stream)++;

                variable = doc_new(name, dt_string, lstring, len);
            }
            else if(**stream == '\n'){                                              // empty value after '='
                variable = doc_new(name, dt_null);
                (*stream)++;
            }
            else{                                                                   // normal value
                lstring = *stream;

                for(*stream = strpbrk(*stream, "\\\n;#"); *stream != NULL && **stream != '\n'; *stream = strpbrk(*stream, "\\\n;#")){

                    if(**stream == ';' || **stream == '#'){                         // jump over inline comments
                        run_whitespace_back(stream);

                        **stream = '\0';
                        (*stream)++;
                        *stream = strpbrk(*stream, "\n");
                        break;
                    }

                    *stream = strpbrk(*stream, "\n");
                    
                    (*stream)++;
                }

                run_whitespace_back(stream);
                (*stream)++;
                **stream = '\0';

                len = *stream - lstring + 1;
                (*stream)++;    

                char *buffer_value = strbreak_clear(lstring);

                variable = doc_from_string(name, buffer_value);

                free(buffer_value);
            }

        }
        else{                                                                           // empty value
            run_whitespace_back(stream);
            (*stream)++;
            **stream = '\0';
            (*stream)++;
            run_whitespace(stream);
            variable = doc_new(name, dt_null);
        }
    }

    return variable;
}

// try and parse a section, variable or anonymous var, and ignore comments
static doc *parse_token(char **stream){
    if(*stream == NULL) return NULL;

    run_whitespace(stream);

    while(**stream == '#' || **stream == ';'){
        *stream = strpbrk(*stream, "\n");
        (*stream)++;
        run_whitespace(stream);
    }

    // if( **stream == '#' || **stream == ';'){                                        // comments
    //     *stream = strpbrk(*stream, "\n");
    //     (*stream)++;
    //     run_whitespace(stream);
    // }

    if( (*stream)[0] == '\0' || (*stream)[1] == '\0') return NULL;

    switch(**stream){
        case '[':                                                                   // section
            return parse_section(stream);
        break;        

        case '{':                                                                   // anonymous variables
        default:                                                                    // variables
            return parse_variable(stream);
        break;
    }
}

// parse a ini file
static doc *parse_ini(char **stream){
    if(*stream == NULL) return NULL;

    doc *ini = doc_new("ini", dt_obj, ";");
    doc *current_section = NULL;

    for(doc *token = parse_token(stream); token != NULL; token = parse_token(stream)){
        if(token->type == dt_obj){                                                  // a section
            doc_append(ini, ".", token);
            current_section = token;
        }
        else{
            if(current_section == NULL){                                            // no specified section yet
                doc_append(ini, ".", token);
            }
            else{                                                                   // inside a section
                doc_append(current_section, ".", token);
            }
        }
    }

    return ini;
}



// print to output stream, reallocating it accordingly
static void printf_stringify(char **string_start_address, size_t *length, size_t buffer_size, char *format, ...){
    va_list args;
    va_start(args, format);
    
    size_t token_size = buffer_size + strlen(format);
    char *token = calloc(token_size, sizeof(*token));
    vsnprintf(token, token_size, format, args);

    *length += strlen(token);
    *string_start_address = realloc(*string_start_address, *length);
    strcat(*string_start_address, token);

    free(token);
    va_end(args);
}

// print a value
static void print_value(doc *variable, char **stream, size_t *length){
    char *buffer;
    size_t buffer_size;
    
    switch(variable->type){
        case dt_double:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, FLOAT_MAX_DECIMAL_CHARS_INI, "{" decimal_print_format "}\n", FLOAT_DECIMAL_PLACES_INI, ((doc_double*)variable)->value);
            else
                printf_stringify(stream, length, FLOAT_MAX_DECIMAL_CHARS_INI + strlen(variable->name), "%s=" decimal_print_format "\n", variable->name, FLOAT_DECIMAL_PLACES_INI, ((doc_double*)variable)->value);
        break;
        case dt_float:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, FLOAT_MAX_DECIMAL_CHARS_INI, "{%*.G}\n", FLOAT_DECIMAL_PLACES_INI, ((doc_float*)variable)->value);
            else
                printf_stringify(stream, length, FLOAT_MAX_DECIMAL_CHARS_INI + strlen(variable->name), "%s=%*.G\n", variable->name, FLOAT_DECIMAL_PLACES_INI, ((doc_float*)variable)->value);
        break;
        
        case dt_uint:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI, "{%u}\n", ((doc_uint_t*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI + strlen(variable->name), "%s=%u\n", variable->name, ((doc_uint_t*)variable)->value);
        break;
        case dt_uint64:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI, "{%u}\n", ((doc_uint64_t*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI + strlen(variable->name), "%s=%u\n", variable->name, ((doc_uint64_t*)variable)->value);
        break;
        case dt_uint32:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI, "{%u}\n", ((doc_uint32_t*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI + strlen(variable->name), "%s=%u\n", variable->name, ((doc_uint32_t*)variable)->value);
        break;
        case dt_uint16:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI, "{%u}\n", ((doc_uint16_t*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI + strlen(variable->name), "%s=%u\n", variable->name, ((doc_uint16_t*)variable)->value);
        break;
        case dt_uint8:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI, "{%u}\n", ((doc_uint8_t*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI + strlen(variable->name), "%s=%u\n", variable->name, ((doc_uint8_t*)variable)->value);
        break;
        
        case dt_int:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI, "{%i}\n", ((doc_int*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI + strlen(variable->name), "%s=%i\n", variable->name, ((doc_int*)variable)->value);
        break;
        case dt_int64:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI, "{%i}\n", ((doc_int64_t*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI + strlen(variable->name), "%s=%i\n", variable->name, ((doc_int64_t*)variable)->value);
        break;
        case dt_int32:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI, "{%i}\n", ((doc_int32_t*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI + strlen(variable->name), "%s=%i\n", variable->name, ((doc_int32_t*)variable)->value);
        break;
        case dt_int16:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI, "{%i}\n", ((doc_int16_t*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI + strlen(variable->name), "%s=%i\n", variable->name, ((doc_int16_t*)variable)->value);
        break;
        case dt_int8:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI, "{%i}\n", ((doc_int8_t*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_INI + strlen(variable->name), "%s=%i\n", variable->name, ((doc_int8_t*)variable)->value);
        break;

        case dt_null:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, 1, "{}\n");
            else
                printf_stringify(stream, length, 1 + strlen(variable->name), "%s=\n", variable->name);
        break;
        case dt_bool:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, 5, "{%s}\n", (((doc_bool*)variable)->value ? "true" : "false"));
            else
                printf_stringify(stream, length, 5 + strlen(variable->name), "%s=%s\n", variable->name, (((doc_bool*)variable)->value ? "true" : "false"));
        break;
        
        case dt_string:
        case dt_const_string:
            if(*variable->name != '\0' && strpbrk(((doc_string*)variable)->string, "#;") != NULL){
                if(*(variable->name) == '\0')
                    printf_stringify(stream, length, ((doc_string*)variable)->len, "{\"%s\"}\n", ((doc_string*)variable)->string);
                else
                    printf_stringify(stream, length, ((doc_string*)variable)->len + strlen(variable->name), "%s=\"%s\"\n", variable->name, ((doc_string*)variable)->string);
            }
            else{
                if(*(variable->name) == '\0')
                    printf_stringify(stream, length, ((doc_string*)variable)->len, "{%s}\n", ((doc_string*)variable)->string);
                else
                    printf_stringify(stream, length, ((doc_string*)variable)->len + strlen(variable->name), "%s=%s\n", variable->name, ((doc_string*)variable)->string);
            }
        break;

        case dt_bindata:
        case dt_const_bindata:
            buffer = base64_encode(((doc_bindata*)variable)->data, ((doc_bindata*)variable)->len); 
            buffer_size = strlen(buffer);
            
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, buffer_size + 1, "{%s}\n", buffer);
            else
                printf_stringify(stream, length, buffer_size + 1 + strlen(variable->name), "%s=%s\n", variable->name, buffer);

            free(buffer);
        break;
    }
}

// stringify to ini
static char *stringify(doc *variable, char **stream, size_t *length){
    switch(variable->type){
        case dt_obj:
        case dt_array:
            printf_stringify(stream, length, strlen(variable->name), "[%s]\n", variable->name);

            for(doc_loop(member, variable))
                stringify(member, stream, length);
            
        break;

        default:
            print_value(variable, stream, length);
        break;
    }
}

/* ----------------------------------------- Functions -------------------------------------- */

// parses a ini/cfg text file into a doc structure 
doc *doc_ini_parse(char *ini_file_stream){
    if(ini_file_stream == NULL) return NULL;

    size_t stream_size = strlen(ini_file_stream);
    char *stream = (char*)calloc(stream_size + 2, sizeof(char));                    // make a copy before passing to parsers(destructive)
    char *base_stream = stream;
    memcpy(stream, ini_file_stream, stream_size);
    stream[stream_size] = '\n';
    stream[stream_size + 1] = '\0';

    doc *doc_ini = parse_ini(&stream);

    free(base_stream);
    return doc_ini;
}

char *doc_ini_stringify(doc *doc_ini){
    char *ini_stream = calloc(1, sizeof(*ini_stream));
    size_t ini_stream_len = 1;

    if(doc_ini->type != dt_obj)
        return NULL;

    doc_squash(doc_ini, ".", 2);

    for(doc_loop(member, doc_ini)){
        stringify(member, &ini_stream, &ini_stream_len);
    }

    return ini_stream;
}

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "doc_ini.h"
#include "parse_utils.h"
#include "base64.h"

/* ----------------------------------------- Private Functions ------------------------------ */

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

        variable = create_doc_from_string("", lstring);

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

                variable = create_doc_from_string(name, buffer_value);

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
                printf_stringify(stream, length, FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "{" decimal_print_format_parse_utils "}\n", FLOAT_DECIMAL_PLACES_PARSE_UTILS, ((doc_double*)variable)->value);
            else
                printf_stringify(stream, length, FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS + strlen(variable->name), "%s=" decimal_print_format_parse_utils "\n", variable->name, FLOAT_DECIMAL_PLACES_PARSE_UTILS, ((doc_double*)variable)->value);
        break;
        case dt_float:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, "{" decimal_print_format_parse_utils "}\n", FLOAT_DECIMAL_PLACES_PARSE_UTILS, ((doc_float*)variable)->value);
            else
                printf_stringify(stream, length, FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS + strlen(variable->name), "%s=%*.G\n", variable->name, FLOAT_DECIMAL_PLACES_PARSE_UTILS, ((doc_float*)variable)->value);
        break;
        
        case dt_uint:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "{%u}\n", ((doc_uint_t*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + strlen(variable->name), "%s=%u\n", variable->name, ((doc_uint_t*)variable)->value);
        break;
        case dt_uint64:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "{%u}\n", ((doc_uint64_t*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + strlen(variable->name), "%s=%u\n", variable->name, ((doc_uint64_t*)variable)->value);
        break;
        case dt_uint32:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "{%u}\n", ((doc_uint32_t*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + strlen(variable->name), "%s=%u\n", variable->name, ((doc_uint32_t*)variable)->value);
        break;
        case dt_uint16:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "{%u}\n", ((doc_uint16_t*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + strlen(variable->name), "%s=%u\n", variable->name, ((doc_uint16_t*)variable)->value);
        break;
        case dt_uint8:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "{%u}\n", ((doc_uint8_t*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + strlen(variable->name), "%s=%u\n", variable->name, ((doc_uint8_t*)variable)->value);
        break;
        
        case dt_int:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "{%i}\n", ((doc_int*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + strlen(variable->name), "%s=%i\n", variable->name, ((doc_int*)variable)->value);
        break;
        case dt_int64:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "{%i}\n", ((doc_int64_t*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + strlen(variable->name), "%s=%i\n", variable->name, ((doc_int64_t*)variable)->value);
        break;
        case dt_int32:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "{%i}\n", ((doc_int32_t*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + strlen(variable->name), "%s=%i\n", variable->name, ((doc_int32_t*)variable)->value);
        break;
        case dt_int16:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "{%i}\n", ((doc_int16_t*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + strlen(variable->name), "%s=%i\n", variable->name, ((doc_int16_t*)variable)->value);
        break;
        case dt_int8:
            if(*(variable->name) == '\0')
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "{%i}\n", ((doc_int8_t*)variable)->value);
            else
                printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + strlen(variable->name), "%s=%i\n", variable->name, ((doc_int8_t*)variable)->value);
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

// opens and parse a ini file to a doc structure
doc *doc_ini_open(char *filename){
    if(filename == NULL){
        return NULL;
    }

    char *stream = fstream(filename);

    doc *ini = doc_ini_parse(stream);

    free(stream);

    return ini;
}

// save doc ini to file
void doc_ini_save(doc *ini_doc, char *filename){
    char *ini = doc_ini_stringify(ini_doc);

    if(ini == NULL) return;

    FILE *out = fopen(filename, "w+");
    fprintf(out, ini);
    fclose(out);
    free(ini);
}

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

// stringify a ini file from a doc strucuture
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

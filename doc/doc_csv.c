#include "doc_csv.h"
#include "parse_utils.h"
#include "../base64/base64.h"
#include <string.h>

#define CSV_SEPARATORS ",;"

/* ----------------------------------------- Private Functions ------------------------------ */

// tokenize a string by delimiters
static char *strtok_csv(char *string, char *delimiters){
    static char *last_string = NULL;
    static char *last_token = NULL;
    char *buf;

    if(string != last_string){
        last_string = string;
        last_token = string;
    }
    if(last_token == NULL){
        return NULL;
    }
    if(*last_token == '\0'){
        buf = last_token;
        last_token = NULL;
        return buf;
    }

    for(size_t i = 0; i < strlen(last_token); i++){
        for(size_t j = 0; j < strlen(delimiters); j++){
            if(last_token[i] == delimiters[j]){
                last_token[i] = '\0';
                buf = last_token;
                last_token += (i + 1);
                return buf;                
            }
        }

        if(i == (strlen(last_token) - 1)){
            buf = last_token;
            last_token = NULL;
            return buf;
        }
    }

    return NULL;
}

// parse a csv line
static doc *parse_line(char **stream){
    char *end_l = strpbrk(*stream, "\r\n");
    bool end_l_null_terminated = false;

    if(end_l == NULL && **stream == '\0'){
        return NULL;
    }
    else if(end_l == NULL && **stream != '\0'){
        end_l_null_terminated = true;
    }
    else if(**stream == '\0'){
        return NULL;
    }
    else{
        *end_l = '\0';

        if(end_l[1] == '\n'){
            end_l[1] = '\0';
            end_l++;
        }
    }

    doc *line = doc_new("", dt_obj, ";");
    doc *cell;

    for(char *token = strtok_csv(*stream, CSV_SEPARATORS); token != NULL; token = strtok_csv(*stream, CSV_SEPARATORS)){
        cell = create_doc_from_string("", token);
        doc_append(line, ".", cell);
    }

    if(end_l_null_terminated)
        *stream = strrchr(*stream, '\0');
    else
        *stream = end_l + 1;

    return line;
}

// parse a csv file
static doc *csv_parse(char *stream, va_list args){
    if(stream == NULL) return NULL;
    
    doc_csv_parse_opt_t options = va_arg(args, doc_csv_parse_opt_t);

    doc *csv = doc_new("", dt_obj, ";");

    for(doc *line = parse_line(&stream); line != NULL; line = parse_line(&stream)){
        doc_append(csv, ".", line);
    }

    switch(options){
        case parse_csv_first_line_as_names:
            {
                doc *name_columns = doc_copy(csv, ".[0]");
                doc_delete(csv, ".[0]");

                for(doc_loop(line, csv)){
                    doc_size_t i = 0;
                    for(doc_loop(cell, line)){
                        // [%u]\0
                        char name_pos[UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + 3];
                        snprintf(name_pos, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + 3, "[%u]", i);
                        char *column_name = doc_get(name_columns, name_pos, char*);
                        doc_rename(cell, ".", column_name);

                        i++;
                    }
                }
            }
        break;

        case parse_csv_first_column_as_names:
            for(doc_loop(line, csv)){
                doc_rename(line, ".", doc_get(line, "[0]", char*));
                doc_delete(line, "[0]");                
            }
        break;

        case (parse_csv_first_line_as_names | parse_csv_first_column_as_names):
            {
                doc *name_columns = doc_copy(csv, "[0]");
                doc_delete(csv, "[0]");

                for(doc_loop(line, csv)){
                    doc_rename(line, ".", doc_get(line, "[0]", char*));
                    doc_delete(line, "[0]");                

                    doc_type_t i = 0;
                    for(doc_loop(cell, line)){
                        char name_pos[UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + 3] = {0};
                        snprintf(name_pos, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS + 3, "[%u]", i + 1);

                        doc_rename(cell, ".", doc_get(name_columns, name_pos, char*));

                        i++;
                    }
                }
            }
        break;
    }

    return csv;
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
            printf_stringify(stream, length, FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, decimal_print_format_parse_utils, FLOAT_DECIMAL_PLACES_PARSE_UTILS, ((doc_double*)variable)->value);
        break;
        case dt_float:
            printf_stringify(stream, length, FLOAT_MAX_DECIMAL_CHARS_PARSE_UTILS, decimal_print_format_parse_utils, FLOAT_DECIMAL_PLACES_PARSE_UTILS, ((doc_float*)variable)->value);
        break;
        
        case dt_uint:
            printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%u", ((doc_uint_t*)variable)->value);
        break;
        case dt_uint64:
            printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%u", ((doc_uint64_t*)variable)->value);
        break;
        case dt_uint32:
            printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%u", ((doc_uint32_t*)variable)->value);
        break;
        case dt_uint16:
            printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%u", ((doc_uint16_t*)variable)->value);
        break;
        case dt_uint8:
            printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%u", ((doc_uint8_t*)variable)->value);
        break;
        
        case dt_int:
            printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%i", ((doc_int*)variable)->value);
        break;
        case dt_int64:
            printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%i", ((doc_int64_t*)variable)->value);
        break;
        case dt_int32:
            printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%i", ((doc_int32_t*)variable)->value);
        break;
        case dt_int16:
            printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%i", ((doc_int16_t*)variable)->value);
        break;
        case dt_int8:
            printf_stringify(stream, length, UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%i", ((doc_int8_t*)variable)->value);
        break;

        case dt_null:
        break;

        case dt_bool:
            printf_stringify(stream, length, 5, "%s", (((doc_bool*)variable)->value ? "true" : "false"));
        break;
        
        case dt_string:
        case dt_const_string:
            printf_stringify(stream, length, ((doc_string*)variable)->len, "%s", ((doc_string*)variable)->string);
        break;

        case dt_bindata:
        case dt_const_bindata:
            buffer = base64_encode(((doc_bindata*)variable)->data, ((doc_bindata*)variable)->len); 
            buffer_size = strlen(buffer);
            
            printf_stringify(stream, length, buffer_size + 1, "%s", buffer);

            free(buffer);
        break;
    }
}

// stringify csv
static char *stringify(doc *csv_doc, va_list args){

    if(csv_doc == NULL) return NULL;
    if(csv_doc->type != dt_obj && csv_doc->type != dt_array) return NULL;

    doc_size_t columns = csv_doc->child->childs;
    for(doc_loop(line, csv_doc)){
        if(line->childs != columns) return NULL;                                          // check if lines have same number of cells

        for(doc_loop(cell, line)){
            if(cell->type == dt_obj || cell->type == dt_array)  return NULL;        // check if cell values are not objects or arrays
        }
    }

    doc_csv_stringify_opt_t options = va_arg(args, doc_csv_stringify_opt_t);

    char *stream = (char*)calloc(1, sizeof(char));
    size_t length = 1;

    if(options & stringify_csv_put_columns_names_in_first_line){
        if(options & stringify_csv_put_line_name_in_first_column){
            printf_stringify(&stream, &length, 1, ",");    
        }
        
        for(doc_loop(column, csv_doc->child)){
            printf_stringify(&stream, &length, strlen(column->name) + 1, "%s", column->name);    

            if(column->next != NULL)
                printf_stringify(&stream, &length, 1, ",");    
        }

        if(csv_doc->child->next != NULL)
            printf_stringify(&stream, &length, 1, "\n");    
    }

    for(doc_loop(line, csv_doc)){
        if(options & stringify_csv_put_line_name_in_first_column){
            printf_stringify(&stream, &length, strlen(line->name) + 1, "%s,", line->name);    
        }

        for(doc_loop(cell,line)){

            print_value(cell, &stream, &length);

            if(cell->next != NULL)
                printf_stringify(&stream, &length, 1, ",");    
        }

        if(line->next != NULL)
            printf_stringify(&stream, &length, 1, "\n");    
    }

    return stream;
}

/* ----------------------------------------- Functions -------------------------------------- */

// save doc csv to file
void doc_csv_save(doc *csv_doc, char *filename, ...){
    va_list args;
    va_start(args, filename);

    char *csv = stringify(csv_doc, args);

    if(csv == NULL) return;

    FILE *out = fopen(filename, "w+");
    fprintf(out, csv);
    fclose(out);
    free(csv);
    va_end(args);
}

// makes a csv stream out of a doc data structure 
char *doc_csv_stringify(doc *csv_doc, ...){
    va_list args;
    va_start(args, csv_doc);

    char *csv = stringify(csv_doc, args);

    va_end(args);

    return csv;
}

// open and parse a csv file by filename
doc *doc_csv_open(char *filename, ...){
    if(filename == NULL) return NULL;

    va_list args;
    va_start(args, filename);

    char *file = fstream(filename);
    if(file == NULL) return NULL;
    
    doc *variable = csv_parse(file, args);

    free(file);
    va_end(args);
    
    return variable;
}

// parse a csv file to a doc structure
doc *doc_csv_parse(char *stream, ...){
    va_list args;
    va_start(args, stream);

    doc *csv = csv_parse(stream, args);

    va_end(args);

    return csv;
}
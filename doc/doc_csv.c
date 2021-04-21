#include "doc_csv.h"
#include "parse_utils.h"
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

    if(end_l == NULL){
        return NULL;
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

    *stream = end_l + 1;

    return line;
}

// parse a csv file
static doc *csv_parse(char *stream, va_list args){
    if(stream == NULL) return NULL;
    
    doc_parse_csv_opt_t options = va_arg(args, doc_parse_csv_opt_t);

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

/* ----------------------------------------- Functions -------------------------------------- */

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
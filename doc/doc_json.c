#include "doc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define VALUE_TOKEN_SEQ ("/**/\"")

char *parse_string(char **string){

    (*string) = strpbrk((*string), "\"");                                           // locate string beggining

    char *special = (*string) - 2;                                                  // +2 because of -=2 inside do while
    char *last_special = (*string);
    
    do{                                                                             // jump over all \" inside the string
        special += 2;
        last_special = special;
        special = strstr(special, "\\\"");
    }while(special != NULL);

    char *end_ptr = strpbrk(last_special, "\"");                                    // get the termianator " character
    size_t span = end_ptr - (*string);

    char *read_string = malloc(sizeof(read_string)*span);

    strncpy(read_string, (*string), span);

    read_string[span] = '\0';

    (*string) = last_special;                                                       // make passed pointer point to after the string read

    return read_string;
}

char *parse_value(char **string){
    char *value_begin = strpbrk((*string), ":");

    value_begin = strpbrk((*string), "\"")
}

doc *doc_parse_json(char *file_stream){
    char *cursor = file_stream;
    doc *variable = malloc(sizeof(variable));
    variable->parent = NULL;
    variable->child = NULL;
    variable->next = NULL;
    variable->prev = NULL;
    variable->name = NULL;
    variable->type = dt_null;

    cursor = strpbrk(file_stream, "{");                                 
    cursor = strpbrk(file_stream, "\"}");

    if(*cursor == '}')
        return variable;

    char *name = parse_string(&cursor);
    variable->name = name;



} 

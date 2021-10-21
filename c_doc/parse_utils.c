#include "parse_utils.h"

/* ----------------------------------------- Globals ---------------------------------------- */

const char *NUMBER_ALPHABET         = "0123456789";
const char *NUMBER_INTEGER_ALPHABET = "0123456789-+";
const char *NUMBER_DECIMAL_ALPHABET = "0123456789.,eE-+";

/* ----------------------------------------- Private Functions ------------------------------ */

// run whitespace except for the '\n' line break
void run_space(char **stream){
    while( **stream != '\n' && ((**stream >= '\b' && **stream <= '\r') || **stream == 32) )
        (*stream)++;
}

// run all whitespace chars
void run_whitespace(char **stream){
    while( ((**stream >= '\b' && **stream <= '\r') || **stream == 32) )
        (*stream)++;
}

// run whitespace backwards
void run_whitespace_back(char **stream){
    while( ((**stream >= '\b' && **stream <= '\r') || **stream == 32) )
        (*stream)--;
}

// check if a string if formed by just members of a defined alphabet 
bool str_alphabet(char *string, char *alphabet){
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
size_t strnchr(char *string, char chr){
    size_t count = 0;

    for(size_t i = 0; i < strlen(string); i++){
        if(string[i] == chr)
            count++;
    }

    return count;
}

// check if string represents a number
bool str_is_number(char *string){
    run_whitespace(&string);

    if(strpbrk(string, NUMBER_ALPHABET) == NULL)                                    // check if there are decimals digits
        return false;

    if( (*string < '0' && *string > '9') && *string != '+' && *string != '-')       // if start is sign or number
        return false;

    if( strnchr(string, '-') > 1 || 
        strnchr(string, '+') > 1 || 
        strnchr(string, '.') > 1 ||
        strnchr(string, ',') > 1 ||
        strnchr(string, 'e') > 1 ||
        strnchr(string, 'E') > 1
    )                                                                               // if contains more than one adiotional char
        return false;

    return true;
}

// check the value of a string representation of the value
doc_type_t check_value_type(char *value){
    if(!strcmp(value, "true") || !strcmp(value, "false")){                          // boolean
        return dt_bool;
    }
    else if(str_alphabet(value, (char *)NUMBER_INTEGER_ALPHABET)){                  // integer                                  
        if(str_is_number(value))
            return integer_dt_type_parse_utils;
        else
            return dt_string;
    }
    else if(str_alphabet(value, (char *)NUMBER_DECIMAL_ALPHABET)){                  // decimal                                  
        if(str_is_number(value))
            return decimal_dt_type_parse_utils;
        else
            return dt_string;
    }
    else{                                                                           // string
        return dt_string;
    }
}

// create a doc from a string with an appropriate value type
doc *create_doc_from_string(char *name, char *value_string){
    doc_type_t type = check_value_type(value_string);
    doc *variable;
    bool boolean_buf;

    switch(type){
        case decimal_dt_type_parse_utils:
            variable = doc_new(name, decimal_dt_type_parse_utils, strto_rational_parse_utils(value_string, NULL));
        break;

        case integer_dt_type_parse_utils:
            variable = doc_new(name, integer_dt_type_parse_utils, strto_integer_parse_utils(value_string, NULL));
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
            variable = doc_new(name, dt_string, value_string, (doc_size_t)(strlen(value_string) + 1));
        break;  
    }

    return variable;
}

// crop and join a space inside a string
void strcrop(char *string, char *start, char *end){
    *start = '\0';
    strcat(string, end);
}

// clean a string with line breaking sequence
char *strbreak_clear(char *string){
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

// creates a ASCII stream from a file
char *fstream(char *filename){
    FILE *file_p = fopen(filename, "r+b");

    fseek(file_p, 0, SEEK_END);
    long file_p_size = ftell(file_p) + 1;
    fseek(file_p, 0, SEEK_SET);

    char *file_stream = (char*)calloc(file_p_size, sizeof(char));
    fread(file_stream, sizeof(char), file_p_size, file_p);
    fclose(file_p);

    return file_stream;
}

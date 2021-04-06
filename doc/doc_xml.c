#include "doc_xml.h"

/* ----------------------------------------- Definitions ------------------------------------ */

#define WHITESPACE                  " \t\n\r\v\f"                                   // white space chars

#define UINT64_MAX_DECIMAL_CHARS    (20)                                            // decimal digits of max uint64 number, 64/log2(10) = 63/3.322 = 20
#define FLOAT_MAX_DECIMAL_CHARS     (27)                                            // decimal digits of max float number, 1.428571428571428492127e-01 is the biggest
#define FLOAT_DECIMAL_PLACES        (10)                                            // 1.428571428571428492127e-01 has 21 decimal places

// data type for rational javascript numbers
#define RATIONAL_TYPE (dt_double)                                                   // to be put on 'type' member of 'doc'
#define strto_rational(const_char_ptr_string, const_char_ptr_ptr_endptr) strtod(const_char_ptr_string, const_char_ptr_ptr_endptr)               // to convert to number
typedef doc_double      rational_number_t;                                          // to allocate the correct 'doc_*' type
                                                                                    
// data type for integer javascript numbers
#define INTEGER_TYPE (dt_int32)                                                     // to be put on 'type' member of 'doc'
#define strto_integer(const_char_ptr_string, const_char_ptr_ptr_endptr) atoi(const_char_ptr_string)                                             // to convert to number
typedef doc_int32_t     integer_number_t;                                           // to allocate the correct 'doc_*' type

/* ----------------------------------------- Private functions ------------------------------ */

void run_whitespace(char **stream){
    while((**stream >= '\b' && **stream <= '\r') || **stream == 0x32)
        *stream++;
}

void vprintf_stringify(char **string_start_address, size_t *length, size_t buffer_size, char *format, va_list args){
    size_t token_size = buffer_size + strlen(format);
    char *token = calloc(token_size, sizeof(*token));
    vsnprintf(token, token_size, format, args);

    *length += strlen(token);
    *string_start_address = realloc(*string_start_address, *length);
    strcat(*string_start_address, token);

    free(token);
}

void printf_stringify(char **string_start_address, size_t *length, size_t buffer_size, char *format, ...){
    va_list args;
    va_start(args, format);
    vprintf_stringify(string_start_address, length, buffer_size, format, args);
    va_end(args);
}

/* ----------------------------------------- Parser ----------------------------------------- */

doc *parse_atribute(char **stream){
    char *name = *stream;
    char *marker = strpbrk(*stream, "=");
    *marker = '\0';
    marker++;

    char *value = strpbrk(marker, "\"");
    value++;
    marker = strpbrk(value, "\"");
    *marker = '\0';

    doc *atribute = doc_new(name, dt_string, value, marker - value + 1);

    marker++;
    *stream = marker;

    return atribute;
}

doc *parse_tag(char **stream){
    char *cursor = *stream;

    cursor = strpbrk(cursor, "<");
    if(cursor == NULL) return NULL;

    cursor++;

    doc *tag;

    char *marker;
    bool self_terminated = false;
    bool last_atribute = false;

    int i = 0;
    while(1){                                                                       // run through atributes
        marker = strpbrk(cursor, WHITESPACE ">/");
        
        if(*marker == '>'){
            last_atribute = true;
        }
        else if(*marker == '/'){
            self_terminated = true;
            last_atribute = true;
        } 

        if(i == 0){
            *marker = '\0';
            tag = doc_new(cursor, dt_obj, ";");                                     // cursor is the tag name                   
            if(!last_atribute)                                                      // if only the name is present, dont add atributes
                doc_add(tag, ".", "atributes", dt_obj, ";");
            marker++;
            *stream = marker;
        }
        else{                                                                       // atributes
            run_whitespace(stream);
            doc *atribute = parse_atribute(stream);
            doc_append(tag, "atributes", atribute);
        }

        i++;

        cursor = marker;
        if(last_atribute && self_terminated){
            *stream = strpbrk(*stream, ">");
            (*stream)++;
            cursor = *stream;
        }
        if(last_atribute) break;
    }   

    marker = strpbrk(cursor, "<");

    if(marker[1] == '/'){                                                           // tag is a value
        *marker = '\0';
        marker++;
        doc_add(tag, ".", "value", dt_string, cursor, marker - cursor + 1);
        if(!self_terminated){
            *stream = strpbrk(marker, ">");
        }
        else{
            marker--;
            *marker = '<';
            *stream = marker;
        }
    }
    else{                                                                           // tag is a object
        while(1){                                                                   // run inside tag looking for closing tag
            marker = strpbrk(*stream, "<");
            *stream = marker;

            if(marker[1] == '/'){
                if(!self_terminated)
                    (*stream) = strpbrk(*stream, ">");

                break;
            } 

            doc *child_tag = parse_tag(stream);
            doc_append(tag, ".", child_tag);
        }
    }

    return tag;                              
}

doc *parse_xml(char **stream){
    if((*stream)[0] == '<' && (*stream)[1] == '?'){                                 // find next tag after xml info
        (*stream)++;
        *stream = strpbrk(*stream, "<"); 
    }

    doc *xml = doc_new("xml", dt_obj, ";");
    doc *tag;

    for(tag = parse_tag(stream); tag != NULL; tag = parse_tag(stream)){             // parse through the tags
        doc_append(xml, ".", tag);
    }

    return xml;
}

doc *doc_xml_parse(char *xml_stream){
    size_t stream_size = strlen(xml_stream);
    char *stream = (char*)calloc(stream_size + 1, sizeof(char));
    char *base_stream = stream;
    memcpy(stream, xml_stream, stream_size);
    stream[stream_size] = '\0';

    doc *doc_xml = parse_xml(&stream);

    free(base_stream);
    return doc_xml;
}

/* ----------------------------------------- Stringifier ------------------------------------ */

void printf_stringify_value(char **base_address, size_t *length, doc *variable, bool use_tags){
    char *buffer = NULL;

    char *rational_format          = (use_tags) ? "<%s>%.*G</%s>" : "%.*G";
    char *integer_format           = (use_tags) ? "<%s>%i</%s>"   : "%i";
    char *unsigned_integer_format  = (use_tags) ? "<%s>%u</%s>"   : "%u";
    char *string_format            = (use_tags) ? "<%s>%s</%s>"   : "%s";

    switch(variable->type){
        case dt_double:
            printf_stringify(base_address, length, 2*strlen(variable->name), rational_format, variable->name, FLOAT_DECIMAL_PLACES, ((doc_double *)(variable))->value, variable->name);
        break;
            
        case dt_float:
            printf_stringify(base_address, length, 2*strlen(variable->name), rational_format, variable->name, FLOAT_DECIMAL_PLACES, ((doc_float *)(variable))->value, variable->name);
        break;

        case dt_int:
            printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS, integer_format, variable->name, ((doc_int *)(variable))->value, variable->name);
        break;
 
        case dt_int8:
            printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS, integer_format, variable->name, ((doc_int8_t *)(variable))->value, variable->name);
        break;
 
        case dt_int16:
            printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS, integer_format, variable->name, ((doc_int16_t *)(variable))->value, variable->name);
        break;
 
        case dt_int32:
            printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS, integer_format, variable->name, ((doc_int32_t *)(variable))->value, variable->name);
        break;
 
        case dt_int64:        
            printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS, integer_format, variable->name, ((doc_int64_t *)(variable))->value, variable->name);
        break;
 
        case dt_uint:
            printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS, unsigned_integer_format, variable->name, ((doc_uint_t *)(variable))->value, variable->name);
        break;
 
        case dt_uint8:
            printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS, unsigned_integer_format, variable->name, ((doc_uint8_t *)(variable))->value, variable->name);
        break;
 
        case dt_uint16:
            printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS, unsigned_integer_format, variable->name, ((doc_uint16_t *)(variable))->value, variable->name);
        break;
 
        case dt_uint32:
            printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS, unsigned_integer_format, variable->name, ((doc_uint32_t *)(variable))->value, variable->name);
        break;
 
        case dt_uint64:
            printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS, unsigned_integer_format, variable->name, ((doc_uint64_t *)(variable))->value, variable->name);
        break;

        case dt_bool:
            printf_stringify(base_address, length, 2*strlen(variable->name) + 5, string_format, variable->name, (((doc_bool *)(variable))->value) ? "true" : "false", variable->name);
        break;

        case dt_null:
            printf_stringify(base_address, length, 2*strlen(variable->name) + 4, string_format, variable->name, "null", variable->name);
        break;

        case dt_string:
        case dt_const_string:
            printf_stringify(base_address, length, 2*strlen(variable->name) + ((doc_string *)variable)->len, string_format, variable->name, ((doc_string *)variable)->string, variable->name);
        break;

        case dt_bindata:
        case dt_const_bindata:
            buffer = base64_encode(((doc_bindata *)variable)->data, ((doc_bindata *)variable)->len);            
            printf_stringify(base_address, length, 2*strlen(variable->name) + strlen(buffer), string_format, variable->name, buffer, variable->name);
            free(buffer);
        break;
    }
}

void xml_stringify(doc *variable, char **base_address, size_t *length){

    char *value = NULL;
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

            // put the tag and atributes
            printf_stringify(base_address, length, strlen(variable->name), "<%s", variable->name);

            doc *atributes = doc_get_ptr(variable, "atributes");
            if(doc_error_code != errno_doc_value_not_found && atributes != NULL){   
                for(doc_ite(atribute, atributes)){
                    printf_stringify(base_address, length, strlen(atribute->name) + ((doc_string*)atribute)->len, " %s=\"%s\"", atribute->name, ((doc_string*)atribute)->string);
                }
            }

            printf_stringify(base_address, length, 1, ">");

            // if contains a single <value> tag, then its a value tag, otherwise is a tag that contains other tags
            doc *value = doc_get_ptr(variable, "value");
            if(variable->childs <= 2 && value != NULL){                             // value tag
                printf_stringify_value(base_address, length, variable, false);
            }
            else{                                                                   // parent tag
                for(doc_ite(member, variable)){
                    xml_stringify(member, base_address, length);
                }
            }

            // close tag
            printf_stringify(base_address, length, strlen(variable->name), "<\%s>", variable->name);

        break;

        default:
            printf_stringify_value(base_address, length, variable, true);
        break;
    }
}

char *doc_xml_stringify(doc *xml_doc){
    char *xml_stream = NULL;
    size_t xml_stream_len = 0;

    if(xml_doc->type != dt_obj)
        return NULL;

    xml_stringify(xml_doc, &xml_stream, &xml_stream_len);

    return xml_stream;
}

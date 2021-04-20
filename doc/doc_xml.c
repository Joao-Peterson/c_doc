#include "doc_xml.h"
#include "parse_utils.h"

/* ----------------------------------------- Private functions ------------------------------ */

static void vprintf_stringify(char **string_start_address, size_t *length, size_t buffer_size, char *format, va_list args){
    size_t token_size = buffer_size + strlen(format);
    char *token = calloc(token_size, sizeof(*token));
    vsnprintf(token, token_size, format, args);

    *length += strlen(token);
    *string_start_address = realloc(*string_start_address, *length);
    strcat(*string_start_address, token);

    free(token);
}

static void printf_stringify(char **string_start_address, size_t *length, size_t buffer_size, char *format, ...){
    va_list args;
    va_start(args, format);
    vprintf_stringify(string_start_address, length, buffer_size, format, args);
    va_end(args);
}

/* ----------------------------------------- Parser ----------------------------------------- */

// parse a single value string
static doc *parse_value(char **stream){
    run_whitespace(stream);
    char *value = *stream;

    *stream = strpbrk(*stream, "<");                                                // find the closing tag or nother tag
    (*stream)--;
    run_whitespace_back(stream);
    (*stream)++;
    char hold = (**stream);
    (**stream) = '\0';

    doc *variable = create_doc_from_string("", value);

    (**stream) = hold;

    return variable;
}

// parse a atribute inside the tag
static doc *parse_atribute(char **stream){
    char *name = *stream;
    char *marker = strpbrk(*stream, "=");
    *marker = '\0';
    marker++;

    char *value = strpbrk(marker, "\"");
    value++;
    marker = strpbrk(value, "\"");
    *marker = '\0';

    doc *atribute = create_doc_from_string(name, value);

    marker++;
    *stream = marker;

    return atribute;
}

// parse a tag from stream
static doc *parse_tag(char **stream){
    char *name = *stream;

    name = strpbrk(name, "<");
    if(name == NULL) return NULL;
    name++;

    doc *tag;
    char *marker;
    bool self_terminated = false;
    bool last_atribute = false;

    *stream = strpbrk(name, WHITESPACE_PARSE_UTILS ">/");                                        // get name and create doc
    char hold_char = **stream;
    (**stream) = '\0';
    tag = doc_new(name, dt_obj, ";");                               

    if(hold_char == '/'){
        (*stream)++;
        *stream = strpbrk(*stream, ">");
        (*stream)++;
        self_terminated = true;
    }
    else if(hold_char != '>'){
        (*stream)++;
        doc_add(tag, ".", "atributes", dt_obj, ";");
        while(1){                                                                   // run through atributes
            run_whitespace(stream);
            if(**stream == '>'){                                                    // no atributes, just the tag name
                (*stream)++;
                break;
            }
            else if(**stream == '/'){                                               // closing tag without a opening tag
                self_terminated = true;
                doc_delete(tag, "atributes");
                *stream = strpbrk(*stream, ">");
                (*stream)++;
                return tag;
            }
            else{                                                                   // with atributes
                doc *atribute = parse_atribute(stream);
                doc_append(tag, "atributes", atribute);
            }
        }   
    }
    else{
        (*stream)++;
    }

    doc *member;

    while(1){                                                                       // run and parse values and other tags inside the tag 
        run_whitespace(stream);

        if((**stream) == '<'){                                                      // tag value
            if(!self_terminated && (*stream)[1] == '/'){                            // found the closing tag
                *stream = strpbrk(*stream, ">");
                (*stream)++;
                return tag;
            }                             
            
            member = parse_tag(stream);
        }
        else{                                                                       // value
            member = parse_value(stream);
        }

        doc_append(tag, ".", member);

        if(self_terminated) return tag;
    }                             
}

// parse a xml file
static doc *parse_xml(char **stream){
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

// parse all elements inside a xml file
doc *doc_xml_parse(char *xml_stream){
    size_t stream_size = strlen(xml_stream);
    char *stream = (char*)calloc(stream_size + 1, sizeof(char));                    // make a copy before passing to parsers(destructive)
    char *base_stream = stream;
    memcpy(stream, xml_stream, stream_size);
    stream[stream_size] = '\0';

    doc *doc_xml = parse_xml(&stream);

    free(base_stream);
    return doc_xml;
}

// opens and parse a xml file to a doc structure
doc *doc_xml_open(char *filename){
    if(filename == NULL){
        return NULL;
    }

    char *stream = fstream(filename);

    doc *xml = doc_xml_parse(stream);

    free(stream);

    return xml;
}

/* ----------------------------------------- Stringifier ------------------------------------ */

// reallocate a output stream and concatenate strings to it 
static void printf_stringify_value(char **base_address, size_t *length, doc *variable, bool use_tags){
    char *buffer = NULL;

    switch(variable->type){
        case dt_double:
            if(use_tags)
                printf_stringify(base_address, length, 2*strlen(variable->name), "<%s>%.*G</%s>", variable->name, FLOAT_DECIMAL_PLACES_PARSE_UTILS, ((doc_double *)(variable))->value, variable->name);
            else
                printf_stringify(base_address, length, 2*strlen(variable->name), "%.*G", FLOAT_DECIMAL_PLACES_PARSE_UTILS, ((doc_double *)(variable))->value);
        break;
            
        case dt_float:
            if(use_tags)
                printf_stringify(base_address, length, 2*strlen(variable->name), "<%s>%.*G</%s>", variable->name, FLOAT_DECIMAL_PLACES_PARSE_UTILS, ((doc_float *)(variable))->value, variable->name);
            else
                printf_stringify(base_address, length, 2*strlen(variable->name), "%.*G", FLOAT_DECIMAL_PLACES_PARSE_UTILS, ((doc_float *)(variable))->value);
        break;

        case dt_int:
            if(use_tags)
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "<%s>%i</%s>", variable->name, ((doc_int *)(variable))->value, variable->name);
            else
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%i", ((doc_int *)(variable))->value);
        break;
 
        case dt_int8:
            if(use_tags)
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "<%s>%i</%s>", variable->name, ((doc_int8_t *)(variable))->value, variable->name);
            else
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%i", ((doc_int8_t *)(variable))->value);
        break;
 
        case dt_int16:
            if(use_tags)
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "<%s>%i</%s>", variable->name, ((doc_int16_t *)(variable))->value, variable->name);
            else
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%i", ((doc_int16_t *)(variable))->value);
        break;
 
        case dt_int32:
            if(use_tags)
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "<%s>%i</%s>", variable->name, ((doc_int32_t *)(variable))->value, variable->name);
            else
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%i", ((doc_int32_t *)(variable))->value);
        break;
 
        case dt_int64:        
            if(use_tags)
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "<%s>%i</%s>", variable->name, ((doc_int64_t *)(variable))->value, variable->name);
            else
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%i", ((doc_int64_t *)(variable))->value);
        break;
 
        case dt_uint:
            if(use_tags)
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "<%s>%u</%s>" , variable->name, ((doc_uint_t *)(variable))->value, variable->name);
            else
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%u", ((doc_uint_t *)(variable))->value);
        break;
 
        case dt_uint8:
            if(use_tags)
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "<%s>%u</%s>" , variable->name, ((doc_uint8_t *)(variable))->value, variable->name);
            else
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%u", ((doc_uint8_t *)(variable))->value);
        break;
 
        case dt_uint16:
            if(use_tags)
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "<%s>%u</%s>" , variable->name, ((doc_uint16_t *)(variable))->value, variable->name);
            else
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%u", ((doc_uint16_t *)(variable))->value);
        break;
 
        case dt_uint32:
            if(use_tags)
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "<%s>%u</%s>" , variable->name, ((doc_uint32_t *)(variable))->value, variable->name);
            else
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%u", ((doc_uint32_t *)(variable))->value);
        break;
 
        case dt_uint64:
            if(use_tags)
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "<%s>%u</%s>" , variable->name, ((doc_uint64_t *)(variable))->value, variable->name);
            else
                printf_stringify(base_address, length, 2*strlen(variable->name) + UINT64_MAX_DECIMAL_CHARS_PARSE_UTILS, "%u", ((doc_uint64_t *)(variable))->value);
        break;

        case dt_bool:
            if(use_tags)
                printf_stringify(base_address, length, 2*strlen(variable->name) + 5, "<%s>%s</%s>", variable->name, (((doc_bool *)(variable))->value) ? "true" : "false", variable->name);
            else
                printf_stringify(base_address, length, 2*strlen(variable->name) + 5, "%s", (((doc_bool *)(variable))->value) ? "true" : "false");
        break;

        case dt_null:
            if(use_tags)
                printf_stringify(base_address, length, 2*strlen(variable->name) + 4, "<%s>%s</%s>", variable->name, "null", variable->name);
            else
                printf_stringify(base_address, length, 2*strlen(variable->name) + 4, "%s", "null");
        break;

        case dt_string:
        case dt_const_string:
            if(use_tags)
                printf_stringify(base_address, length, 2*strlen(variable->name) + ((doc_string *)variable)->len, "<%s>%s</%s>", variable->name, ((doc_string *)variable)->string, variable->name);
            else
                printf_stringify(base_address, length, 2*strlen(variable->name) + ((doc_string *)variable)->len, "%s", ((doc_string *)variable)->string);
        break;

        case dt_bindata:
        case dt_const_bindata:
            buffer = base64_encode(((doc_bindata *)variable)->data, ((doc_bindata *)variable)->len);            

            if(use_tags)
                printf_stringify(base_address, length, 2*strlen(variable->name) + strlen(buffer), "<%s>%s</%s>", variable->name, buffer, variable->name);
            else
                printf_stringify(base_address, length, 2*strlen(variable->name) + strlen(buffer), "%s", buffer);

            free(buffer);
        break;
    }
}

// recursive call for generating text based on doc structure
static void stringify(doc *variable, char **base_address, size_t *length){

    char *value = NULL;
    doc *member = NULL;
    size_t value_len;
    bool first_call = false;
    bool atributes_tag_jump = false;

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
                for(doc_loop(atribute, atributes)){
                    printf_stringify(base_address, length, strlen(atribute->name), " %s=\"", atribute->name);
                    printf_stringify_value(base_address, length, atribute, false);
                    printf_stringify(base_address, length, 1, "\"");
                }
            }

            printf_stringify(base_address, length, 1, ">");

            // generate values and other tags inside the tag
            for(doc_loop(member, variable)){
                if(!atributes_tag_jump && !strcmp(member->name, "atributes")){      // if the child isn't the atributes object
                    atributes_tag_jump = true;
                    continue;
                }
                    
                if(!strcmp(member->name, "")){
                    printf_stringify_value(base_address, length, member, false);
                }
                else{
                    stringify(member, base_address, length);
                }
            }

            // close tag
            printf_stringify(base_address, length, strlen(variable->name), "</%s>", variable->name);

        break;

        default:
            printf_stringify_value(base_address, length, variable, true);
        break;
    }
}

// main call for xml stringify 
char *doc_xml_stringify(doc *xml_doc){
    char *xml_stream = NULL;
    size_t xml_stream_len = 0;

    if(xml_doc->type != dt_obj)
        return NULL;

    for(doc_loop(member, xml_doc)){
        stringify(member, &xml_stream, &xml_stream_len);
    }

    return xml_stream;
}

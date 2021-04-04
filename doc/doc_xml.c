#include "doc.h"
#include "doc_xml.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define WHITESPACE " \t\n\r\v\f"

void run_whitespace(char **stream){
    while((**stream >= '\b' && **stream <= '\r') || **stream == 0x32)
        *stream++;
}

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

doc *doc_parse_xml(char *xml_stream){
    size_t stream_size = strlen(xml_stream);
    char *stream = (char*)calloc(stream_size + 1, sizeof(char));
    char *base_stream = stream;
    memcpy(stream, xml_stream, stream_size);
    stream[stream_size] = '\0';

    doc *doc_xml = parse_xml(&stream);

    free(base_stream);
    return doc_xml;
}
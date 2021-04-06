#ifndef _DOC_XML_HEADER_
#define _DOC_XML_HEADER_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include "doc.h"
#include "../base64/base64.h"

/* ----------------------------------------- Functions -------------------------------------- */

/**
 * @brief parse a xml file stream into a doc structure
 * @param xml_stream: the stream of the file in memory
 * @return a doc data structure, where each tag that contains other tags will be a
 * dt_obj in wich the sub tags will be it's childs, tags that have values will be 
 * a dt_obj varaible with a child named value with type dt_string, 
 * even number and other data types will follow this format.
 * Each tag also has atributes, this atributes will be packed inside a variable named atributes,
 * of type dt_obj, inside the tag object where its childs will be dt_string with the atributes.
 */
doc *doc_xml_parse(char *xml_stream);

char *doc_xml_stringify(doc *xml_doc);

#endif
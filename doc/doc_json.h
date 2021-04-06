#ifndef _DOC_JSON_HEADER_
#define _DOC_JSON_HEADER_

#include "doc.h"

/* ----------------------------------------- Functions -------------------------------------- */

/**
 * @brief parse a file stream to a 'doc' structure
 * @param file_stream: null terminated file stream
 * @return pointer to 'doc' structure
 */
doc *doc_json_parse(char *file_stream);

/**
 * @brief creates a json file out of a 'doc' structure
 * @param json_doc: pointer to 'doc' structure
 * @return json char stream, null terminated. Return NULL if json_doc is not a dt_obj 'doc' type
 * @note json_doc must be of type dt_obj, as if it represents the actual json file,
 * and the name will not be displayed, because json files start with a '{'
 */
char *doc_json_stringify(doc *json_doc);

#endif
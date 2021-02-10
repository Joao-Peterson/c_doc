#ifndef _DOC_JSON_HEADER_
#define _DOC_JSON_HEADER_

#include "doc.h"

doc *doc_parse_json(char *file_stream);

char *doc_stringify_json(doc *json_doc);

#endif
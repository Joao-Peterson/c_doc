#ifndef _DOC_INI_HEADER_
#define _DOC_INI_HEADER_

#include "doc.h"

/* ----------------------------------------- Functions -------------------------------------- */

/**
 * @brief parses a ini/cfg text file into a doc structure
 * @note supports non nesting sections, line break sequence '\', string literals like: "string",
 * simple variables, empty variables like: 'var=' and 'var ' without the '=' sign, anonymous variables, 
 * ex: '{anonymous_variable}', have to be surrounded by curly brackets and will have a "" empty name string,
 * 'doc_get()' and 'doc_get_ptr()' with a syntax like: 'anonymous_variables[1]' must be used, getting the value by index.
 * Comments use the '#' or ';' characters.     
 * Variables must terminate with a line break
 * @param ini_file_stream: a string of ASCII chars in memory
 * @return the doc data structure representing the file 
 */
doc *doc_ini_parse(char *ini_file_stream);

char *doc_ini_stringify(doc *doc_ini);

#endif
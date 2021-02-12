#ifndef _DOC_JSON_HEADER_
#define _DOC_JSON_HEADER_

#include "doc.h"

/* ----------------------------------------- Definitions ------------------------------------ */

#define VALUE_TOKEN_SEQ             ("\"-0123456789.{[tfn")                         // to find any value type, string, array, obj, number, bool or null
#define VALUE_TOKEN_SEQ_W_SQR_BRK   ("\"-0123456789.{[tfn]")                        // same as above but with closing sqr brk, for anonymous members in arrays
#define TERMINATORS                 (",}]")                                         // to check end of member, obj or array
#define WHITE_SPACE                 (" \t\n\r\v\f")                                 // white space chars

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

/* ----------------------------------------- Functions -------------------------------------- */

/**
 * @brief parse a file stream to a 'doc' structure
 * @param file_stream: null terminated file stream
 * @return pointer to 'doc' structure
 */
doc *doc_parse_json(char *file_stream);

/**
 * @brief creates a json file out of a 'doc' structure
 * @param json_doc: pointer to 'doc' structure
 * @return json char stream, null terminated. Return NULL if json_doc is not a dt_obj 'doc' type
 * @note json_doc must be of type dt_obj, as if it represents the actual json file,
 * and the name will not be displayed, because json files start with a '{'
 */
char *doc_stringify_json(doc *json_doc);

#endif
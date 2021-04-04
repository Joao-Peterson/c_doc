#ifndef _DOC_PRINT_HEADER_
#define _DOC_PRINT_HEADER_

#include "doc.h"
#include <stdio.h>
#include <stdbool.h>

typedef int (*fprint_function_t) (FILE* file, const char *control, ...);
typedef int (*print_function_t)  (const char *control, ...);

void doc_print_set(print_function_t print_function_ptr);

void doc_print_file_set(fprint_function_t fprint_function_ptr, FILE *f_out);

void doc_print(doc *variable);

#endif
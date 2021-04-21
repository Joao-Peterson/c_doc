#ifndef _DOC_CSV_HEADER_
#define _DOC_CSV_HEADER_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include "doc.h"

/* ----------------------------------------- Enumerators ------------------------------------ */

typedef enum{
    parse_csv_normal_mode           = 0,
    parse_csv_first_line_as_names   = (1<<0),
    parse_csv_first_column_as_names = (1<<1)
}doc_parse_csv_opt_t;

/* ----------------------------------------- Functions -------------------------------------- */

doc *doc_csv_open(char *filename, ...);

doc *doc_csv_parse(char *stream, ...);

#endif
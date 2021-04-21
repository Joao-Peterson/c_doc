#include <stdio.h>
#include <stdlib.h>
#include "doc.h"
#include "doc_csv.h"
#include "doc_print.h"

int main(int argc, char **argv){

    // doc *csv = doc_csv_open("test/sample.csv", csv_parse_first_column_as_names | csv_parse_first_line_as_names);
    // doc_print(csv);
    // doc_csv_save(csv, "test/out.csv", csv_stringify_put_columns_names_in_first_line | csv_stringify_put_line_name_in_first_column);

    doc *pl = doc_csv_open("test/test.pl");
    // doc_print(pl);
    doc_csv_save(pl, "test/out.pl", csv_stringify_use_custom_separator, ';');

    return 0;
}

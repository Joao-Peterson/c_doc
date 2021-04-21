#include <stdio.h>
#include <stdlib.h>
#include "doc.h"
#include "doc_csv.h"
#include "doc_print.h"

int main(int argc, char **argv){

    doc *csv = doc_csv_open("test/sample.csv", parse_csv_first_column_as_names | parse_csv_first_line_as_names);

    doc_print(csv);

    return 0;
}
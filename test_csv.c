#include <stdio.h>
#include <stdlib.h>
#include "doc.h"
#include "doc_csv.h"
#include "doc_print.h"

int main(int argc, char **argv){

    doc *csv = doc_csv_open("test/sample.csv", parse_csv_first_column_as_names | parse_csv_first_line_as_names);

    doc_print(csv);

    char *file = doc_csv_stringify(csv, stringify_csv_put_line_name_in_first_column | stringify_csv_put_columns_names_in_first_line);
    // char *file = doc_csv_stringify(csv);

    if(file != NULL){
        FILE *out = fopen("test/out.csv", "w+");
        fprintf(out, file);
        fclose(out);
    }

    return 0;
}

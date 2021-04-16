#include <stdio.h>
#include <stdlib.h>
#include "doc/doc.h"
#include "doc/doc_ini.h"
#include "doc/doc_json.h"
#include "doc/doc_print.h"

char *fstream(char *filename){
    FILE *file_p = fopen(filename, "r+b");

    fseek(file_p, 0, SEEK_END);
    long file_p_size = ftell(file_p) + 1;
    fseek(file_p, 0, SEEK_SET);

    char *file_stream = (char*)calloc(file_p_size, sizeof(char));
    fread(file_stream, sizeof(char), file_p_size, file_p);
    fclose(file_p);

    return file_stream;
}

int main(int argc, char **argv){

    char *json_stream = fstream("test/squash.json");

    if(json_stream == NULL) return 0;

    doc *json = doc_json_parse(json_stream);

    doc_squash(json, ".", 2);

    doc_print_file_set(fprintf, stdout);
    doc_print(json);

    return 0;
}
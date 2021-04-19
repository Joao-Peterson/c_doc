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

    char *ini_stream = fstream("test/config.ini");
    if(ini_stream == NULL) return 0;

    doc *ini = doc_ini_parse(ini_stream);

    doc_print_file_set(fprintf, stdout);
    doc_print(ini);

    char *ini_out_stream = doc_ini_stringify(ini);

    FILE *ini_out = fopen("test/ini_out.ini", "w+b");
    fwrite(ini_out_stream, 1, strlen(ini_out_stream), ini_out);
    fclose(ini_out); 

    free(ini_out_stream);
    free(ini_stream);
    doc_delete(ini, ".");

    return 0;
}
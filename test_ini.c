#include <stdio.h>
#include <stdlib.h>
#include "doc/doc.h"
#include "doc/doc_ini.h"
#include "doc/doc_print.h"

int main(int argc, char **argv){

    FILE *ini_file = fopen("test/config.ini", "r+b");

    fseek(ini_file, 0, SEEK_END);
    long ini_file_size = ftell(ini_file) + 1;
    fseek(ini_file, 0, SEEK_SET);

    char *ini_stream = (char*)calloc(ini_file_size, sizeof(char));
    fread(ini_stream, sizeof(char), ini_file_size, ini_file);
    fclose(ini_file);

    doc *ini = doc_ini_parse(ini_stream);

    free(ini_stream);

    doc_print(ini);

    return 0;
}
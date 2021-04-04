#include <stdlib.h>
#include <stdio.h>

#include "doc.h"
#include "doc_xml.h"
#include "doc_print.h"

char *read_asci(const char *path, size_t *size_ptr){
    FILE *file = fopen(path, "r"); //abre arquivo
    if(file==NULL){ // verifica se ponteiro lido é nulo
        printf("Arquivo nao encontrado ou invalido\n");
        return NULL;
    }

    fseek(file,0,SEEK_END); // vai até o fim do file
    long len = ftell(file); // pega tamanho
    fseek(file,0,SEEK_SET); // volta ao inicio

    char *buf = (char*)malloc(sizeof(char)*(len+1)); //aloca buffer com tamanho +1
    fread(buf,sizeof(char),len,file); // lê para o buffer
    buf[len]='\0'; // introduz fechamento de string

    fclose(file);
    *size_ptr = len;
    return buf;
}

int main(int argc, char **argv){

    size_t len;
    char *xml_stream = read_asci("test/books.xml", &len);

    doc *xml = doc_parse_xml(xml_stream);

    doc_print(xml);

    doc_delete(xml, ".");

    return 0;
}
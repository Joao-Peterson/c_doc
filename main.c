#include <stdio.h>
#include <stdlib.h>
#include "doc/doc.h"
#include "doc/doc_json.h"

char *read_asci(const char *path){
    FILE *file = fopen(path, "r"); //abre arquivo
    if(file==NULL){ // verifica se ponteiro lido é nulo
        printf("Arquivo nao encontrado ou invalido\n");
        return NULL;
    }

    fseek(file,0,SEEK_END); // vai até o fim do file
    int len = ftell(file); // pega tamanho
    fseek(file,0,SEEK_SET); // volta ao inicio

    char *buf = (char*)malloc(sizeof(char)*(len+1)); //aloca buffer com tamanho +1
    fread(buf,sizeof(char),len,file); // lê para o buffer
    buf[len]='\0'; // introduz fechamento de string

    fclose(file);
    return buf;
}

int main(int argc, char **argv){

    char *json_stream = read_asci("./test/types.json");
    doc *json_doc = doc_parse_json(json_stream);

    char *json_stream_out = doc_stringify_json(json_doc);    

    FILE *json_out = fopen("./test/out.json", "w+");
    fprintf(json_out, "%s", json_stream_out);
    fflush(json_out);
    fclose(json_out);

    doc_delete(json_doc, ".");
    free(json_stream);
    free(json_stream_out);

    return 0;
}   
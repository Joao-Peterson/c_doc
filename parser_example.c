#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "doc/doc.h"
#include "doc/doc_json.h"
#include "base64/base64.h"

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

    char blob[] = "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.";

    char *json_stream = read_asci("./test/types.json"); 
    doc *json_doc = doc_parse_json(json_stream);

    doc_add(json_doc, ".", "blob", dt_const_bindata, (void *)blob, (size_t)269);

    if(doc_error_code){
        printf("[DOC] : %s\n", doc_get_error_msg());
    }

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
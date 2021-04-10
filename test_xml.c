#include <stdlib.h>
#include <stdio.h>

#include "doc.h"
#include "doc_xml.h"
#include "doc_print.h"

char *read_asci(const char *path, size_t *size_ptr){
    FILE *file = fopen(path, "r+b"); 
    if(file==NULL){ 
        printf("file invalid or not found\n");
        return NULL;
    }

    fseek(file,0,SEEK_END); 
    long len = ftell(file); 
    fseek(file,0,SEEK_SET); 

    char *buf = (char*)calloc((len+1), sizeof(char)); 
    fread(buf,sizeof(char),len,file);
    buf[len]='\0'; 

    fclose(file);
    *size_ptr = len;
    return buf;
}

int main(int argc, char **argv){

    size_t len;
    char *xml_stream = read_asci("test/tinsul_tampas.xml", &len);

    doc *xml = doc_xml_parse(xml_stream);

    // doc_print(xml);

    char *xml_string = doc_xml_stringify(xml);

    FILE *xml_out = fopen("test/out.xml", "w+");

    fprintf(xml_out, xml_string);

    free(xml_string);
    fclose(xml_out);
    doc_delete(xml, ".");

    return 0;
}
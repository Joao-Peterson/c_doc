#include "base64.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

const char *base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char base64_padding = '=';

char *base64_encode(void *data, size_t data_len){
    uint8_t *data_bytes = data;
    size_t i = 0;
    size_t j = 0;
    size_t to_process = 0;
    char *base64 = calloc( (data_len / 3)*3 + 1 + 3 + 1, sizeof(*base64));     
    uint8_t temp_bytes[3] = {0};
    char temp_char;

    if(data_len >= 3)
        to_process = 3;
    else
        to_process = data_len;

    while(1){
        if(to_process % 3 == 0){
            base64[j+0] = base64_chars[((data_bytes[i+0] & 0xFC) >> 2)];
            base64[j+1] = base64_chars[((data_bytes[i+0] & 0x03) << 4) | ((data_bytes[i+1] & 0xF0) >> 4)];
            base64[j+2] = base64_chars[((data_bytes[i+1] & 0x0F) << 2) | ((data_bytes[i+2] & 0xC0) >> 6)];
            base64[j+3] = base64_chars[((data_bytes[i+2] & 0x2F) << 0)];
        }
        else{
            for(int z = 0; z < data_len; z++)
                temp_bytes [z] = data_bytes[i+z];

            base64[j+0] = base64_chars[((temp_bytes[0] & 0xFC) >> 2)];
            base64[j+1] = base64_chars[((temp_bytes[0] & 0x03) << 4) | ((temp_bytes[1] & 0xF0) >> 4)];

            temp_char = base64_chars[((temp_bytes[1] & 0x0F) << 2) | ((temp_bytes[2] & 0xC0) >> 6)];
            if(temp_char == base64_chars[0] && temp_bytes[1] == 0 && temp_bytes[2] == 0)
                base64[j+2] = base64_padding;
            else
                base64[j+2] = base64_chars[((temp_bytes[1] & 0x0F) << 2) | ((temp_bytes[2] & 0xC0) >> 0)];
            
            temp_char = base64_chars[((temp_bytes[2] & 0x2F) << 0)];
            if(temp_char == base64_chars[0] && temp_bytes[2] == 0)
                base64[j+3] = base64_padding;
            else
                base64[j+3] = base64_chars[((temp_bytes[2] & 0x2F) << 0)];

            base64[j+4] = '\0';
        }
    
        i += 3;
        j += 4;

        if(data_len >= 3)
            data_len -= 3;
        else
            break;

        if(data_len >= 3)
            to_process = 3;
        else
            to_process = data_len;
    }

    return base64;
}

void *base64_decode(char *data, size_t data_len){
    size_t pos_data = 0; 
    size_t pos_content = 0; 
    uint32_t buffer;

    uint8_t *base64_chars_decode = calloc(97+26+1, sizeof(*base64_chars_decode));

    size_t len = ((data_len / 4)*3 + 1);
    uint8_t *content = calloc(len, sizeof(*content));

    for(int x = 65; x < (65+26); x++)
        base64_chars_decode[x] = x - 65;

    for(int x = 97; x < (97+26); x++)
        base64_chars_decode[x] = x - 97;

    for(int x = 48; x < (48+10); x++)
        base64_chars_decode[x] = x - 48;

    base64_chars_decode['+'] = 63;
    base64_chars_decode['/'] = 64;

    while(data_len > 0){

        buffer = 0;

        for(int i = 0; i < 4; i++){
            buffer |= base64_chars_decode[data[pos_data + i]];
            buffer <<= 6;
        }

        for(int i = 3; i > 0; i--){
            content[pos_content + i] = *((uint8_t *)(&buffer) + (i - 1));
        }
        
        data_len -= 4;
        pos_data += 4;
    }

    free(base64_chars_decode);

    return content;
}

#include "base64.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* ----------------------------------------- Private Globals -------------------------------- */

// base64 table indexed by sextet
const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// reverse base64 table, indexed by char whose value is the sextet
const uint8_t base64_chars_decode[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x3F, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33};

// base64 padding
const char base64_padding = '=';

/* ----------------------------------------- Functions -------------------------------------- */

// base64 encode
char *base64_encode(void *data, size_t data_len){
    size_t pos_data = 0; 
    size_t pos_encoded = 0; 
    uint32_t buffer = 0;
    uint8_t *data_bytes = (uint8_t*)data;

    if(data == NULL)
        return NULL;

    /**
     * base64 encoded messages have a 4/3 ratio for output/input, because the radix is 64, 
     * therefore 6 bits are required for representation, a sextect. The minimum commom multiplier is 24,
     * in other words, three bytes of binary data makes four base64 chars.
     */
    char *encoded_data = calloc(1, sizeof(*encoded_data) * ( (data_len / 3 + 1) * 4 + 1));                      
    
    while(pos_data < data_len){

        buffer = 0;

        for(int i = 0; i < 3; i++){                                                 // put three chars from input into buffer, while shifting back
            if((pos_data + i) <= data_len)                                          // fill missing bytes with zeroes
                buffer |= data_bytes[pos_data + i];
            else
                buffer |= 0;

            if(i < 2)
                buffer <<= 8;
        }

        for(int i = 0; i < 4; i++){                                                 // take out four chars while shifting forward
            uint32_t sextet = (uint8_t)( ( buffer & ( 0x3F << 6*(3-i) ) ) >> 6*(3-i) );

            if(((data_len - pos_data) < 3) && sextet == 0)                          // pad the last zeros with '='
                encoded_data[pos_encoded + i] = base64_padding;
            else
                encoded_data[pos_encoded + i] = base64_chars[sextet];
        }

        pos_data += 3;
        pos_encoded += 4;
    }

    return encoded_data;
}

// base64 decode
void *base64_decode(char *data, size_t data_len, size_t *output_data_length){
    size_t pos_data = 0; 
    size_t pos_content = 0; 
    uint32_t buffer;

    if(data == NULL)
        return NULL;

    size_t len = ((data_len / 4)*3 + 1);                                    
    uint8_t *content = calloc(len, sizeof(*content));

    while(pos_data < data_len){

        buffer = 0;

        for(int i = 0; i < 4; i++){                                                 // put base64 chars into buffer while shifting back
            buffer |= base64_chars_decode[data[pos_data + i]];

            if(i < 3)
                buffer <<= 6;
        }

        for(int i = 3; i > 0; i--){                                                 // take out three bytes while shifting forward with pointer arithmetic
            content[pos_content + 3 - i] = *((uint8_t *)(&buffer) + (i - 1));
        }
        
        pos_data += 4;
        pos_content += 3;
    }

    if(output_data_length != NULL)
        *output_data_length = pos_content;

    return content;
}

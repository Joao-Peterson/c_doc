#ifndef _BASE64_HEADER_
#define _BASE64_HEADER_

#include <stdlib.h>

char *base64_encode(void *data, size_t data_len);

void *base64_decode(char *data, size_t data_len);

#endif
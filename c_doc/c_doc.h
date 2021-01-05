#ifndef _C_DOC_
#define _C_DOC_

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

/* ----------------------------------------- Enum's ----------------------------------------- */

typedef enum{
    dt_null,
    dt_obj,
    dt_array,
           
    dt_double,
    dt_float, 
            
    dt_uint,
    dt_uint64,
    dt_uint32,
    dt_uint16,
    dt_uint8,
             
    dt_int,
    dt_int64,
    dt_int32,
    dt_int16,
    dt_int8,   
                 
    dt_bool,  
               
    dt_string, 
    dt_const_string,
              
    dt_bindata,
    dt_const_bindata
}doc_type_t;

// assert macro
#define IS_DOC_TYPE(type) \
   (((type) == dt_null)             || \
    ((type) == dt_obj)              || \
    ((type) == dt_array)            || \
    ((type) == dt_double)           || \
    ((type) == dt_float)            || \
    ((type) == dt_uint)             || \
    ((type) == dt_uint64)           || \
    ((type) == dt_uint32)           || \
    ((type) == dt_uint16)           || \
    ((type) == dt_uint8)            || \
    ((type) == dt_int)              || \
    ((type) == dt_int64)            || \
    ((type) == dt_int32)            || \
    ((type) == dt_int16)            || \
    ((type) == dt_int8)             || \
    ((type) == dt_bool)             || \
    ((type) == dt_string)           || \
    ((type) == dt_const_string)     || \
    ((type) == dt_bindata)          || \
    ((type) == dt_const_bindata))

/* ----------------------------------------- Structs ---------------------------------------- */

// main datatype
typedef struct doc doc;
struct doc{
    doc *next;
    doc *prev;
    doc *child;
    doc_type_t type;
    char *name;
};

// ------------------ individual values datatypes

#pragma pack(push,1)

typedef struct{
    doc header;
    union{
        double value;
        uint8_t bytes[sizeof(double)];
    };
}doc_double;

typedef struct{
    doc header;
    union{
        float value;
        uint8_t bytes[sizeof(float)];
    };
}doc_float;

typedef struct{
    doc header;
    union{
        unsigned int value;
        uint8_t bytes[sizeof(unsigned int)];
    };
}doc_uint;

typedef struct{
    doc header;
    union{
        uint64_t value;
        uint8_t bytes[sizeof(uint64_t)];
    };
}doc_uint64;

typedef struct{
    doc header;
    union{
        uint32_t value;
        uint8_t bytes[sizeof(uint32_t)];
    };
}doc_uint32;

typedef struct{
    doc header;
    union{
        uint16_t value;
        uint8_t bytes[sizeof(uint16_t)];
    };
}doc_uint16;

typedef struct{
    doc header;
    uint8_t value;
}doc_uint8;


typedef struct{
    doc header;
    union{
        int value;
        uint8_t bytes[sizeof(int)];
    };
}doc_int;

typedef struct{
    doc header;
    union{
        int64_t value;
        uint8_t bytes[sizeof(int64_t)];
    };
}doc_int64;

typedef struct{
    doc header;
    union{
        int32_t value;
        uint8_t bytes[sizeof(int32_t)];
    };
}doc_int32;

typedef struct{
    doc header;
    union{
        int16_t value;
        uint8_t bytes[sizeof(int16_t)];
    };
}doc_int16;

typedef struct{
    doc header;
    int8_t value;
}doc_int8;

typedef struct{
    doc header;
    bool value;
}doc_bool;

typedef struct{
    doc header;
    size_t len;
    char *string;
}doc_string;

typedef struct{
    doc header;
    size_t len;
    uint8_t *data;
}doc_bin_data;

#pragma (pop)

/* ----------------------------------------- Prototypes ------------------------------------- */



#endif
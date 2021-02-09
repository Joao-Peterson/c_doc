/**
 * @file doc.h
 * 
 * DOC - The C dynamic object lib!
 * 
 * This library is made for C and C++ programs and implement a data structure to mimic dynamic objects
 * present in interpreted languages, parsers can be written to convert files notations such as xml, json, etc, to
 * the data structure.
 * 
 * Created by: João Peterson Scheffer - 2021.
 * 
 * This is the header file, please don't modify, except when stated in documentation.
 * 
 * Refer to this header as means to use the library and get informations about the variables and functions.
 * 
 */

#ifndef _DOC_HEADER_
#define _DOC_HEADER_

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

/* ----------------------------------------- Definitions ------------------------------------ */

#define ERROR_MSG_LEN_DOC_HEADER    500     // max string len for error msg, used by doc_get_error_msg()

#define DOC_NAME_MAX_LEN            100     // max name length for doc instance

#define MAX_OBJ_MEMBER_QTY   UINT32_MAX     // maximum quantity of members inside a array or object  

/* ----------------------------------------- Typedef's ---------------------------------------- */

typedef uint32_t childs_amount_t;           // type for looping through members inside array's or obj's  

typedef unsigned int uint_t;                // syntax sugar for getting the values with get_value() macro later

/* ----------------------------------------- Enum's ----------------------------------------- */

/**
 * @brief enumerator that contains types for use on the syntax
 */
typedef enum{
    dt_null,                                /**< Denotes a null value with no 'doc_' inherited structure */                                                
    dt_obj,                                 /**< Denotes a object, can hold members */                                            
    dt_array,                               /**< Denotes a array, can hold members only of the same type */                                                
    dt_double,                              /**< Denotes a double value */                                                
    dt_float,                               /**< Denotes a float value */                                                 
    dt_uint,                                /**< Denotes a uint value */                                                
    dt_uint64,                              /**< Denotes a uint64 value */                                                
    dt_uint32,                              /**< Denotes a uint32 value */                                                
    dt_uint16,                              /**< Denotes a uint16 value */                                                
    dt_uint8,                               /**< Denotes a uint8 value */                                                
    dt_int,                                 /**< Denotes a int value */                                            
    dt_int64,                               /**< Denotes a int64 value */                                                
    dt_int32,                               /**< Denotes a int32 value */                                                
    dt_int16,                               /**< Denotes a int16 value */                                                
    dt_int8,                                /**< Denotes a int8 value */                                                   
    dt_bool,                                /**< Denotes a bool value */                                                  
    dt_string,                              /**< Denotes a normal string that can be freed */                                                 
    dt_const_string,                        /**< Denotes a normal string that can't be freed */                                                                      
    dt_bindata,                             /**< Denotes binary data that can be freed */                                                
    dt_const_bindata                        /**< Denotes binary data that can't be freed */                                                       
}doc_type_t;

/**
 * @brief macros for staticaly check type
 */
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

/**
 * @brief main structure of the lib
 */
typedef struct doc doc;
struct doc{
    doc *next;                              /**< pointer to next member inside a array or object */
    doc *prev;                              /**< pointer to previous member inside a array or object*/
    doc *child;                             /**< pointer to the first element of this array or object */
    doc *parent;                            /**< pointer to the parent, the instance that define the object or array */
    childs_amount_t childs;                 /**< quantity of childs */
    doc_type_t type;                        /**< type that describes this instance */
    char *name;                             /**< name of the element */
};

// ------------------ individual values datatypes

#pragma pack(push,1)

/**
 * @brief structure that inherit from doc that holds 'double' data, typecasting from 'doc' to 'doc_double' makes the data visible
 */
typedef struct{
    doc header;                                 /**< struct inherited of */
    union{                                      /**< anonymous union, used for serialization */
        double value;                           /**< actual value */
        uint8_t bytes[sizeof(double)];          /**< array of bytes of the value */
    };
}doc_double;

/**
 * @brief structure that inherit from doc that holds 'float' data, typecasting from 'doc' to 'doc_float' makes the data visible
 */
typedef struct{
    doc header;                                 /**< struct inherited of */        
    union{                                      /**< anonymous union, used for serialization */                        
        float value;                            /**< actual value */                                    
        uint8_t bytes[sizeof(float)];           /**< array of bytes of the value */                                                
    };
}doc_float;

/**
 * @brief structure that inherit from doc that holds 'uint_t' data, typecasting from 'doc' to 'doc_uint_t' makes the data visible
 */
typedef struct{
    doc header;                                 /**< struct inherited of */                                        
    union{                                      /**< anonymous union, used for serialization */                                            
        unsigned int value;                     /**< actual value */                                                            
        uint8_t bytes[sizeof(unsigned int)];    /**< array of bytes of the value */                                                            
    };
}doc_uint_t;

/**
 * @brief structure that inherit from doc that holds 'uint64_t' data, typecasting from 'doc' to 'doc_uint64_t' makes the data visible
 */
typedef struct{
    doc header;                                 /**< struct inherited of */                    
    union{                                      /**< anonymous union, used for serialization */                
        uint64_t value;                         /**< actual value */                            
        uint8_t bytes[sizeof(uint64_t)];        /**< array of bytes of the value */                                                
    };
}doc_uint64_t;

/**
 * @brief structure that inherit from doc that holds 'uint32_t' data, typecasting from 'doc' to 'doc_uint32_t' makes the data visible
 */
typedef struct{
    doc header;                                 /**< struct inherited of */                                
    union{                                      /**< anonymous union, used for serialization */                            
        uint32_t value;                         /**< actual value */                                        
        uint8_t bytes[sizeof(uint32_t)];        /**< array of bytes of the value */                                                            
    };
}doc_uint32_t;

/**
 * @brief structure that inherit from doc that holds 'uint16_t' data, typecasting from 'doc' to 'doc_uint16_t' makes the data visible
 */
typedef struct{
    doc header;                                 /**< struct inherited of */                                                            
    union{                                      /**< anonymous union, used for serialization */                                                        
        uint16_t value;                         /**< actual value */                                                                    
        uint8_t bytes[sizeof(uint16_t)];        /**< array of bytes of the value */                                                                                        
    };
}doc_uint16_t;

/**
 * @brief structure that inherit from doc that holds 'uint8_t' data, typecasting from 'doc' to 'doc_uint8_t' makes the data visible
 */
typedef struct{
    doc header;                                 /**< struct inherited of */                                    
    uint8_t value;                              /**< actual value */             
}doc_uint8_t;

/**
 * @brief structure that inherit from doc that holds 'int_t' data, typecasting from 'doc' to 'doc_int_t' makes the data visible
 */
typedef struct{
    doc header;                                 /**< struct inherited of */                                            
    union{                                      /**< anonymous union, used for serialization */                                        
        int value;                              /**< actual value */                                                
        uint8_t bytes[sizeof(int)];             /**< array of bytes of the value */                                                                
    };
}doc_int;

/**
 * @brief structure that inherit from doc that holds 'int64_t' data, typecasting from 'doc' to 'doc_int64_t' makes the data visible
 */
typedef struct{
    doc header;                                 /**< struct inherited of */                                                        
    union{                                      /**< anonymous union, used for serialization */                                                    
        int64_t value;                          /**< actual value */                                                                
        uint8_t bytes[sizeof(int64_t)];         /**< array of bytes of the value */                                                                                
    };
}doc_int64_t;

/**
 * @brief structure that inherit from doc that holds 'int32_t' data, typecasting from 'doc' to 'doc_int32_t' makes the data visible
 */
typedef struct{
    doc header;                                 /**< struct inherited of */                                                        
    union{                                      /**< anonymous union, used for serialization */                                                    
        int32_t value;                          /**< actual value */                                                                
        uint8_t bytes[sizeof(int32_t)];         /**< array of bytes of the value */                                                                                
    };
}doc_int32_t;

/**
 * @brief structure that inherit from doc that holds 'int16_t' data, typecasting from 'doc' to 'doc_int16_t' makes the data visible
 */
typedef struct{
    doc header;                                 /**< struct inherited of */                                                
    union{                                      /**< anonymous union, used for serialization */                                            
        int16_t value;                          /**< actual value */                                                        
        uint8_t bytes[sizeof(int16_t)];         /**< array of bytes of the value */                                                                        
    };
}doc_int16_t;

/**
 * @brief structure that inherit from doc that holds 'int8_t' data, typecasting from 'doc' to 'doc_int8_t' makes the data visible
 */
typedef struct{
    doc header;                                 /**< struct inherited of */                                                    
    int8_t value;                               /**< actual value */                                           
}doc_int8_t;

/**
 * @brief structure that inherit from doc that holds 'bool' data, typecasting from 'doc' to 'doc_bool' makes the data visible
 */
typedef struct{
    doc header;                                 /**< struct inherited of */                                                                                
    bool value;                                 /**< actual value */                                                                                       
}doc_bool;

/**
 * @brief structure that inherit from doc that holds 'char *' data, typecasting from 'doc' to 'doc_string' makes the data visible
 */
typedef struct{
    doc header;                                 /**< struct inherited of */                                                     
    size_t len;                                 /**< length of the string */                                                            
    char *string;                               /**< pointer to string data */                                                         
}doc_string;

/**
 * @brief structure that inherit from doc that holds 'uint8_t *' data, typecasting from 'doc' to 'doc_bindata' makes the data visible
 */
typedef struct{
    doc header;                                 /**< struct inherited of */                                                    
    size_t len;                                 /**< length of the binary data */                                                   
    uint8_t *data;                              /**< pointer to binary data */                                                         
}doc_bindata;

#pragma (pop)

/* ----------------------------------------- Prototypes ------------------------------------- */

/**
 * @brief check if obj is null, if it is, returns a dummy doc 
 * to be written to by some macros that would segfault on a null pointer
 * @param obj: pointer to doc
 * @return the object itself or a dummy in case obj is null
 */
doc *__check_obj(doc *obj); 

/**
 * @brief check if obj is null and if it is a value type, if it is, returns a dummy doc 
 * to be written to by some macros that would segfault on a null pointer
 * @param obj: pointer to doc
 * @return the object itself or a dummy in case obj is null or non value type
 */
doc *__check_obj_is_value(doc *obj);

/**
 * @brief internal function, visible only for macro porpouses
 * @return error code defined by 'errno_doc_code_t' in 'doc.c'
 */
int __doc_get_error_code(void);

/**
 * @brief internal function, visible only for macro porpouses
 * @return string with the error message
 */
char *doc_get_error_msg(void);

/**
 * @brief creates a new object based on the lib syntax
 * @param name: name of the object
 * @param type: type defined by doc_type_t
 * @param ...: syntax for new data, name followed by the type and associeted value, if any
 * @return pointer to newly created dynamic object
 */
doc *doc_new(char *name, doc_type_t type, ...);

/**
 * @brief adds new data to existing instance
 * @param object_or_array: pointer to existing object
 * @param name_to_add_to: name of the instance inside object_or_array to add to
 * @param name: name of new data
 * @param type: the type of this data
 * @param ...: syntax for new data, name followed by the type and associeted value, if any
 */
void doc_add(doc *object_or_array, char *name_to_add_to, char *name, doc_type_t type, ...);

/**
 * @brief deletes a instance and its members recursevely, if any
 * @param object_or_array: pointer to existing object
 * @param name: name of the data inside object_or_array to delete
 */
void doc_delete(doc *object_or_array, char *name);

/**
 * @brief gets a pointer to element inside object_or_array
 * @param object_or_array: pointer to existing object
 * @param name: name of the data inside object_or_array
 * @return pointer to the element
 */
doc *doc_get(doc* object_or_array, char *name);

/**
 * @brief get the amount of childs an array or obj has
 * @param object_or_array: pointer to existing object
 * @return amount of childs
 */
childs_amount_t doc_childs_amount(doc *object_or_array);

/**
 * @brief set string data pointer and string len
 * @param obj: pointer to existing object
 * @param name: name of the data inside obj
 * @param new_string: new string pointer
 * @param new_len: new string len
 */
void doc_set_string(doc *obj, char *name, char *new_string, size_t new_len);

/**
 * @brief set binary data pointer and its len
 * @param obj: pointer to existing object
 * @param name: name of the data inside obj
 * @param new_data: new data pointer
 * @param new_len: new data len
 */
void doc_set_bindata(doc *obj, char *name, char *new_data, size_t new_len);

/* ----------------------------------------- Macros ----------------------------------------- */

/**
 * @brief gets the actual value from a doc instance
 * @param obj: pointer to the value instance
 * @param type: type of the data, C keyword types
 * @return the actual value
 */
#define doc_get_value(obj, type)    (((doc_##type*)__check_obj(obj))->value)

/**
 * @brief gets the string pointer from a doc instance
 * @param obj: pointer to the string instance
 * @return pointer to string
 */
#define doc_get_string(obj)         ( (((doc_string*)__check_obj(obj))->string) )

/**
 * @brief gets the string len from a doc instance
 * @param obj: pointer to the string instance
 * @return string length
 */
#define doc_get_string_len(obj)     (((doc_string*)__check_obj(obj))->len)

/**
 * @brief gets the binary data len from a doc instance
 * @param obj: pointer to the data instance
 * @return pointer to binary data
 */
#define doc_get_bindata(obj)        (   (__check_obj(obj)->type == dt_bindata) ? (uint8_t*)(((doc_bindata*)__check_obj(obj))->data) : (const uint8_t*)(((doc_bindata*)__check_obj(obj))->data)   )

/**
 * @brief gets the binary data len from a doc instance
 * @param obj: pointer to the data instance
 * @return size_t length   
 * @return binary data length
 */
#define doc_get_bindata_len(obj)    (((doc_bindata*)__check_obj(obj))->len)

/**
 * @brief read only variable to access the error code set by the last call from the lib 
 */
#define doc_error_code (__doc_get_error_code())

/**
 * @brief sets the new value for a instance
 * @param obj: pointer to the data instance
 * @param name: name of the data inside obj
 * @param type: type of the data, C keyword types
 * @param new_value: value to be set
 */
#define doc_set_value(obj, name, type, new_value)  ( ((doc_##type*)__check_obj_is_value(doc_get(__check_obj(obj),name)))->value = new_value )

#endif
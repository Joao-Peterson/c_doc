# DOC - The C dynamic object lib!

This library is made for C and C++ programs and implement a data structure to mimic dynamic objects
present in interpreted languages, parsers can be written to convert files notations such as xml, json, etc, to
the data structure.

TL;DR: Sources and include files are in [doc](./doc) and [base64](./base64). Use doc_new() to create a new object and _set() and _get() commands
to interact, doc_delete() when you're done. Also check out the parser for ease handle of markdown files!

Created by: João Peterson Scheffer - 2021.
Version: v1.4

## Table of contents
* [Compilation](#compilation)
* [Use](#use)
* [Syntax](#syntax)
    * [Value](#values)
    * [Lists](#lists)
* [Functions](#functions)
* [Iteration](#iteration)
* [Error checking](#error-checking)
* [Struct interfaces](#struct-interfaces)
* [Parse and Stringify](#parse-and-stringify)

### Compilation

To compile this library simply compile the source with the header, no dependencies.
There a make file, simply type `make release` and the lib should be compiled to the directory [dist](./dist).

### Use

A sample application is written in [example_doc.c](./example_doc.c).

Starting by creating a new object:

```c    
    doc *obj = doc_new(
        "velocidades", dt_obj,
            "max", dt_double, 56.0,
            "media", dt_double, 22.5,
            "pontos", dt_array,
                "p1", dt_double, 23.0,        
                "p2", dt_double, 70.0,        
                "p3", dt_double, 104.3,        
                "p4", dt_double, 22.0,
            ";",
            "integer", dt_uint8, 255,
            "packets", dt_const_bindata, (void *)some_data_array, (size_t)9,
            "future_value", dt_null,
            "some_object", dt_obj,
            ";",
            "matrix", dt_array, 
                dt_array, 
                    "1", dt_int, 11,
                    "2", dt_int, 22,
                    "3", dt_int, 33,
                ";",
            ";", 
            "some_values", dt_array,
                dt_uint8, 251,
                dt_uint8, 252,
                dt_uint8, 253,
            ";",
        ";"
    ); 
```

The syntax is, name, type and then value or values, if any. Objects and arrays must have a ";" terminator character.
Arrays only hold members of the same type. Members dt_null don't have a value. dt_string, dt_const_string, dt_bindata and
dt_const_bindata have a extra value, the length *size_t*. dt_const_ instances have pointers to const data in memory, so the library
doesn't free its memory when a *doc_delete()* call occurs. 

You can add extra data to existing ones:

```c
    doc_add(obj, ".", 
        "medidas", dt_obj,
            "m1", dt_const_string, "bruh", 5,
            "M1", dt_uint32, 35420,
            "data", dt_const_bindata, "AAAAAAAAAAAAAAAAAAA", 20, 
        ";"
    );
```
Here you can see the second argument ".", reffers to a instance, in this case "." is the instance of the object itself, "velocidades". But you can specify any instance inside, the syntax is "member.member.member.member" and so on. 

You can then get a value using a agnostic type call:

```c
    // get value
    double value = doc_get(obj, "pontos.p3", double);
```

In a similar way you can set the value:

```c
    doc_set(obj, "pontos.p3", double, 130.0);
```

And in the end, delete it all, or any element actually, using the same syntax as seen above in *doc_add()*.

```c
    // delete all, but can be any instance
    doc_delete(obj,".");
```

### Syntax

As showed in the [Use](#use) section, the syntax for defining data into the doc structure can be divided as [values](#values) and [lists](#lists). 

#### Values

Values have a name, a type and a value. Any number type has this syntax:

```c
    doc *example_doc = doc_new("integer", dt_int, 45);
    
    doc *example_doc = doc_new("double", dt_double, 45.0);

    doc *example_doc = doc_new("bool", dt_bool, true);
    // or
    doc *example_doc = doc_new("bool", dt_bool, 1);
```

Strings expect a size value.

```c
    doc *example_doc = doc_new("string", dt_string, "Some string", 12ULL);
    // or 
    doc *example_doc = doc_new("string", dt_string, "Some string", (size_t)12);
```

The size has to be a 64 bit values, as the library expects a *size_t* type value, or unsigned long long, as the integer literal *ULL* suggests.

String can also have constant data, *dt_const_string*, that is, they hold a pointer to *const char* data, when we delete with *doc_delete*, the library will know not to deallocate this data, as with *dt_string*, the data will be copied first. 

```c
    const char *some_const_data = "Const string";
    doc *example_doc = doc_new("string", dt_const_string, some_const_data, 13ULL);
```

We can declare also raw binary data values with (dt_bindata) and (dt_const_bindata), the syntax is the same as string. 

```c
    uint8_t *array = { 0x00, 0x01, 0x02, 0xFF};
    doc *example_doc = doc_new("binary data", dt_bindata, array, 4ULL);
```

Null values also exist.

```c
    doc *example_doc = doc_new("null", dt_null);
```

#### Lists

List are objects and arrays, both are basically the same expect that arrays have typechecking, making for correctness. Also, arrays have anonymous members, although we can name then.

They should have a espace sequence at the end, denoted by *";"*, so the argument list know where to stop parsing.

They can also be defined empty.

```c
    doc *example_doc = doc_new(
        "object", dt_obj,
        ";"
    );
```

```c
    doc *example_doc = doc_new(
        "object", dt_obj,
            "member1", dt_null,
            "member2", dt_float, 56.8,
        ";"
    );
```

```c
    doc *example_doc = doc_new(
        "array", dt_array,
            dt_int, 34,
            dt_int, 24,
            dt_int, 14,
        ";"
    );
```

```c
    doc *example_doc = doc_new(
        "array", dt_array,
            dt_array,
                dt_obj, 
                    "member1", dt_null,
                    "member2", dt_float, 57.8,
                ";",
                dt_obj, 
                    "member1", dt_null,
                    "member2", dt_float, 56.8,
                ";",
            ";",

            dt_array,
                dt_obj, 
                    "member1", dt_null,
                    "member2", dt_float, 55.8,
                ";",
                dt_obj, 
                    "member1", dt_null,
                    "member2", dt_float, 54.8,
                ";",
            ";",
        ";"
    );
```

### Functions

This library implements a basic CRUD interaction, along with add, copy and append. All functions, expect for *doc_new* and the error functions,
need a pointer to the doc data type and and value reference, or value name, kinda like refrering to a member inside a object. Ex:

```c
    "object.value_parent.value_child"
```

Or if you are refeering to the object itself, use a dot.

```c
    "."
```

To create you use *doc_new*, this gives you a pointer to this value, and this pointer should be used to any other calls, as well as delete.

```c
    doc *new_doc = doc_new(
        "object", dt_obj,
            "value", dt_int, 24,
        ";"
    );
```

To read you use *doc_get*, it needs a type to work.

```c
    int value = doc_get(new_doc, "value", int);
```

To update you use *doc_set*, it also needs a type, and a value.

```c
    doc_set(new_doc, "value", int, 12);
```

To delete you use *doc_delete*. It deletes from the reference pointer downwards, recursively.

```c
    doc_delete(new_doc, ".");
```

You can also add more data using the library syntax with *doc_add*. 

```c
    doc_add(new_doc, ".",
         "powers", dt_array,
            dt_int, 1,
            dt_int, 2,
            dt_int, 4,
            dt_int, 8,
            dt_int, 16,
            dt_int, 32,
            dt_int, 64,
        ";"
    );
```

Copy data.

```c
    doc *powers = doc_copy(new_doc, "powers");
```

And append new data.

```c
    doc *append_data = doc_new(
        "data", dt_obj,
            "append_data", dt_string, "Append important data", 22ULL,
        ";"
    );

    doc_append(powers, ".", append_data);
```

You can also get references to values using *doc_get_ptr*

```c
    doc *doc_value = doc_get_ptr(new_doc, "value");
    int value = doc_get(doc_value, ".", int);
```

### Iteration

This library provides a macro for doing iteration over a object or array.

```c
    for(doc_ite(cursor, obj)){
        printf("Member loop: %s\n", cursor->name);
    }
```
The *doc_ite* sits inside the for loop, where *cursor* is the name of the iterator and *obj* the object to be iterated over. Cursor has type
*doc* that is equal to a child of *obj*, therefore we can access the members value, like in the example where we can printf the names of the objects.

### Error checking

You can check errors pretty easily using the *doc_error_code* variable and *doc_get_error_msg()* to get a string message.
Integer 0 means ok, negative numbers are errors, positive numbers are informative messages.

```c
    if(doc_error_code < 0){
        printf("Error: %s",doc_get_error_msg());
        return -1;
    }
```

This enum has all the errors listed.

```c
    typedef enum{
        errno_doc_size_of_string_or_bindata_is_beyond_four_megabytes_Check_if_size_is_of_type_size_t_or_cast_it_to_size_t_first     =  1,
        errno_doc_ok                                                                                                                =  0,
        errno_doc_not_a_type                                                                                                        = -1,
        errno_doc_overflow_quantity_members_or_name_is_too_big                                                                      = -2,
        errno_doc_value_not_same_type_as_array                                                                                      = -3,
        errno_doc_duplicate_names                                                                                                   = -4,
        errno_doc_null_passed_obj                                                                                                   = -5,
        errno_doc_obj_not_found                                                                                                     = -6,
        errno_doc_name_cointains_illegal_characters_or_missing_semi_colon_terminator                                                = -7,
        errno_doc_trying_to_add_new_data_to_non_object_or_non_array                                                                 = -8,
        errno_doc_trying_to_set_value_of_non_value_type_data_type                                                                   = -9,
        errno_doc_trying_to_set_string_of_non_string_data_type                                                                      = -10,
        errno_doc_trying_to_set_bindata_of_non_bindata_data_type                                                                    = -11
    }errno_doc_code_t;
```

### Struct interfaces

An ugly and experimental feature, but sometimes useful, for when you want to quickly add and read data to and from a doc structure from and to a normal struct. 

Check it out, you can set two preprocessor symbols with the name and members of your struct, then include the [doc_struct](./doc/doc_struct.h) header file before your code and it will generate the actual struct and calls at compile time.

```c
    #define STRUCT_NAME struct_ex_t
    #define STRUCT_MEMBERS \
        X(int, value1)\
        X(int, value2)\
        X(double, value3)
    #include "doc_struct.h"
```

Now you can create, read and write to a doc structure from a struct and vice versa using the predefined calls *doc_struct_new_STRUCT_NAME*, *doc_struct_set_STRUCT_NAME* and *doc_struct_get_STRUCT_NAME*.

```c
    // interfaces with structs
    struct_ex_t custom_struct = {.value1 = 20, .value2 = 44, .value3 = 69.0};

    doc *struct_doc = doc_struct_new_struct_ex_t(custom_struct);

    double struct_value = doc_get(struct_doc, "value3", double);
```

```c
    custom_struct.value3 = 75.0;
    doc_struct_set_struct_ex_t(custom_struct, struct_doc);
    struct_value = doc_get(struct_doc, "value3", double);
    printf("struct value3: %f\n", struct_value); 
    // struct value3: 75.000000
```

```c
    doc_set(struct_doc, "value2", int , 100);
    doc_struct_get_struct_ex_t(&custom_struct, struct_doc);
    printf("struct value2: %i\n", custom_struct.value2); 
    // struct value2: 100.000000
```

### Parse and Stringify

The doc data structure has been made with a main idea in mind, ease handling of markdown files, widely used for data transmission, configuration data, encapsulate data, organize, and so on. So it's pretty easy to parse and create files to and from the data structure. 

For a json file format, it goes like this:

```c
    char *json_stream = read_asci("./test/types.json"); 
    doc *json_doc = doc_parse_json(json_stream);
```

First loading the the file into memory, then calling the parser to get the data structure, simple. 

Calling stringify we can can make any data structure into a json file, as long as the parent doc is a single object file.

```c
    char *json_stream_out = doc_stringify_json(json_doc);
```

We can even add new data to parsed files and stringify then with a few lines of code:

```c
    char blob[270] = "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.";

    doc_add(json_doc, ".", "blob", dt_const_bindata, (void *)blob, (size_t)269);
```

In this example we added a long string as constant binary data, without the null terminator character, with any other data type, the strigified version will be exactly the same, except with dt_bindata and dt_const_bindata, where the data will be converted to base64 first.

In the example [parser_example.c](./parser_example.c), types.json looks like this before:

```c
    {
        "string" : "string_alou",
        "rational_number" : 1.25,
        "integer_number" : 64,
        "null_value" : null,
        "bool" : true,
        "array_crazy_numbers" : [ 1.0, 3.14E-1, 100E10]
    }
```

And after:

```c
    {
        "string": "string_alou",
        "rational_number": 1.25,
        "integer_number": 64,
        "null_value": null,
        "bool": true,
        "array_crazy_numbers": [1, 0.314, 1E+12],
        "blob": "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4="
    }
```

The actual output does not contain line breaks and identation, this is a prettyfied version.
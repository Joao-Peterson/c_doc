# DOC - The C dynamic object lib!

This library is made for C and C++ programs and implement a data structure to mimic dynamic objects
present in interpreted languages, parsers can be written to convert files notations such as xml, json, etc, to
the data structure.

TL;DR: Sources and include files are in [doc](./doc) and [base64](./base64). Use doc_new() to create a new object and _set() and _get() commands
to interact, doc_delete() when you're done. Also check out the parser for ease handle of markdown files!

Created by: João Peterson Scheffer - 2021.
Version: v1.0

### Compilation

To compile this library simply compile the source with the header, no dependencies.
Arquive if static compile or compile as shared and you can use in your projects, or as i like, compile the source into a object
and link it with your main application in compilation or linking step.

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
dt_const_bindata have a extra value, the length (size_T). dt_const_ instances have pointers to const data in memory, so the library
doesn't free its memory when a doc_delete() call occurs. 

You can check errors pretty easily using the doc_error_code variable and doc_get_error_msg() to get a string message.
0 means ok, negative numbers are errors, positive numbers are informative messages.

```c
    if(doc_error_code < 0){
        printf("Error: %s",doc_get_error_msg());
        return -1;
    }
```
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

You can then get a pointer to the value and retrieve the value using a agnostic type call:

```c
    // get
    doc *ptr = doc_get(obj, "pontos.p3");

    // retrive
    double value = doc_get_value(ptr, double); 
```

In a similar way you can set the value:

```c
    doc_set(obj, "pontos.p3", double, 130.0);
```

And in the end, delete it all, or any element actually, using the same syntax as seen above in doc_add().

```c
    // delete all, but can be any instance
    doc_delete(obj,".");
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

In this example we added a long string as constant binary data, without the null terminator character (269), with any other data type, the strigified version will be exactly the same, except with dt_bindata and dt_const_bindata, where the data will be converted to base64 first.

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
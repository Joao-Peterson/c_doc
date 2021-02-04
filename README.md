# DOC - The C dynamic object lib!

This library is made for C and C++ programs and implement a data structure to mimic dynamic objects
present in interpreted languages, parsers can be written to convert files notations such as xml, json, etc, to
the data structure.

TL;DR: Sources and include files are in [doc](./doc). Use doc_new() to create a new object and _set() and _get() commands
to interact, doc_delete() when you're done. Also check out the parser for ease handle of markdown files!

Created by: João Peterson Scheffer - 2021.
Version: v1.0

### Compilation

To compile this library simply compile the source with the header, no dependencies.
Arquive if static compile or compile as shared and you can use in your projects, or as i like, compile the source into a object
and link it with your main application in compilation or linking step.

### Use

A sample application is written in [main.c](./main.c).

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
            "packets", dt_const_bindata, "void_ptr", 9,
            "future_value", dt_null,
            "some_object", dt_obj,
            ";",
        ";"
    ); 
```

The syntax is, name, type and then value or values, if any. Objects and arrays must have a ";" terminator character.
Arrays only hold members of the same type. Members dt_null don't have a value. dt_string, dt_const_string, dt_bindata and
dt_const_bindata have a extra value, the length. dt_const_ instances have pointers to const data in memory, so the library
doesn't free its memory when a doc_delete() call occurs. 

You can check errors pretty easily using the doc_error_code variable and doc_get_error_msg() to get a string message.

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
    doc_set_value(obj, "pontos.p3", double, 130.0);
```

And in the end, delete it all, or any element actually, using the same syntax as seen above in doc_add().

```c
    // delete all, but can be any instance
    doc_delete(obj,".");
```


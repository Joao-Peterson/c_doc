#ifndef _STATIC_ASSERT__
#define _STATIC_ASSERT__

/* 
    If the macro USE_STATIC_ASSERT is defined, then define static_assert(cond,msg).
    this macro, static_assert, tries to declare a static int array, with size 1 or -1 depending on the condition,
    id the cond is true, then the array has size 1, otherwise is -1 and the compiler throws an error, showing the 
    array's name, wich is static_assertion___##msg, where ##msg will be substituted by the msg variable as text.

    Ex:

    // normla assert with msg
    assert(IS_DOC_TYPE(member_type) && "Type_provided_is_not_an_actual_type!");

    // static assert with msg
    static_assert(IS_DOC_TYPE(member_type), Type_provided_is_not_an_actual_type!);
*/

#ifdef USE_STATIC_ASSERT
    #define static_assert(cond,msg) static int static_assertion__I__##cond__I__##msg[ (cond) ? 1 : -1 ]  
#else
    #define static_assert(cond,msg)
#endif

#endif
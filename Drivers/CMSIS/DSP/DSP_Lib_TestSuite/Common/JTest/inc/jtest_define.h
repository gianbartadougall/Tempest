#ifndef _JTEST_DEFINE_H_
#define _JTEST_DEFINE_H_

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

/**
 *  Makes a symbol for use as a struct name. Names made this way have two parts;
 *  the first parts is a prefix common to all structs of that class. The second
 *  is a specifier which differs for each instance of that struct type.
 */
#define JTEST_STRUCT_NAME(prefix, specifier)    \
    CONCAT(prefix, specifier)

/**
 *  Define a struct with type with a name generated by #JTEST_STRUCT_NAME().
 */
#define JTEST_DEFINE_STRUCT(type, struct_name)    \
    type struct_name

/**
 *  Declare a struct with type with a name generated by #JTEST_STRUCT_NAME().
 */
#define JTEST_DECLARE_STRUCT(struct_definition) \
    extern struct_definition

/**
 *  Define and initialize a struct (created with JTEST_DEFINE_STRUCT()) and
 *  initialize it with init_values.
 */
#define JTEST_INIT_STRUCT(struct_definition, init_values)       \
    struct_definition = {                                       \
        init_values                                             \
    }

#endif /* _JTEST_DEFINE_H_ */

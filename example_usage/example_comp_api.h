// example_comp_api: Example / Demo API implemented by a codebase we want to compartmentalise in a library

#ifndef _EXAMPLE_COMP_API__
#define _EXAMPLE_COMP_API__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Very basic logging in the compartment
#define COMPARTMENT LOG_LEVEL

#ifdef __cplusplus
extern "C" {
#endif

    int32_t example_add_two_numbers(int32_t a, int32_t b);

    char* example_copy_string_to_heap(const char* str);

    bool example_print_heap_string_and_free(char* str, int16_t chars_to_print);

    struct example_struct
    {
        uint32_t i;
        bool b;
        char c;
    };

    void example_dump_struct(const struct example_struct *data);

    bool example_set_compartment_debug_level(int32_t debug_level);

#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_COMP_API__ */

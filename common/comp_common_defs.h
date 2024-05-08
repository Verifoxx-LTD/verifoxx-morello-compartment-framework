// Common definitions needed by compartments and the capability manager

#ifndef _COMP_COMMON_H__
#define _COMP_COMMON_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "example_comp_api.h"
#include "comp_common_asm.h"

#ifdef __cplusplus
extern "C"
{
#endif
    /* Fn Ptr for the unwrap function */
    typedef void(*CompUnwrapFnPtr)(void*);

    /* Fn Ptr for the callback service function */
    typedef uintptr_t(*CompServiceCallbackFnPtr)(void*);

    /* Function pointer typedefs for all Compartment functions called from capability manager
    * C not C++ typedefs
    */
    typedef int32_t(*FnPtr_example_add_two_numbers)(int32_t, int32_t);
    typedef char*(*FnPtr_example_copy_string_to_heap)(const char *);
    typedef bool(*FnPtr_example_print_heap_string_and_free)(char*, int16_t);
    typedef void(*FnPtr_example_dump_struct)(const struct example_struct*);
    typedef bool(*FnPtr_example_set_compartment_debug_level)(int32_t);

    // Declare the initial function in the compartment
    void CompartmentUnwrap(void* comp_data_table);

    // Declare the return function in the compartment
    void CompartmentReturn(CompExitAsmFnPtr fp, uintptr_t return_arg);

#ifdef __cplusplus
}
#endif

#endif /* _COMP_COMMON_H__ */


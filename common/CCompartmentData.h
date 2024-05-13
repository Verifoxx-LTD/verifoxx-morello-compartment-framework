// Copyright (C) 2024 Verifoxx Limited
// CompartmentData classes are used to transfer the arguments for functions to call in the compartment.
// Each compartment API call needs a corresponding class in this file.

#ifndef _COMPARTMENT_DATA_H__
#define _COMPARTMENT_DATA_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "example_comp_api.h"
#include "comp_common_defs.h"
#include "capmgr_service_function_types.h"

// Compartment API Fn Call type - not a C++ enum
typedef enum
{
    CompCall_callExampleAddTwoNumbers,
    CompCall_callExampleCopyStringToHeap,
    CompCall_callExamplePrintHeapStringAndFree,
    CompCall_callExampleDumpStruct,
    CompCall_callExampleSetCompartmentDebugLevel
} CompCall_t;

// Base class for any Compartment Call function data
// Contains needed function pointers and a dervied type ID to determine which underlying function to call
class alignas(__BIGGEST_ALIGNMENT__) CCompartmentData
{
public:
    CompExitAsmFnPtr comp_exit_fp;                  // Function pointer to the return function in the cap manager
    CompEntryAsmFnPtr service_callback_entry_fp;    // Function pointer to the callback service function entry point in the cap manager
    CompServiceCallbackFnPtr capmgr_service_fp;     // Function pointer to the callback service handling function in capability manager
    void* sealer_cap;                               // Capability used by compartment to seal data passed back to capmgr

    void* fp;                                       // Function pointer to the underlying compartment function to call
    CompCall_t       comp_call_type;                // Which derived class it is

    const ServiceFunctionTable* service_func_table; // Table for capability manager service callback functions
public:
    CCompartmentData(CompCall_t call_type) : comp_exit_fp(nullptr), capmgr_service_fp(nullptr), 
        fp(nullptr), comp_call_type(call_type) {}

    virtual ~CCompartmentData() {}
};

// Params for the call example_add_two_numbers()
class alignas(__BIGGEST_ALIGNMENT__) CExampleAddTwoNumbersCallCompartmentData : public CCompartmentData
{
public:
    int32_t a;
    int32_t b;

public:
    CExampleAddTwoNumbersCallCompartmentData(
        int32_t a_,
        int32_t b_
        ) : CCompartmentData(CompCall_callExampleAddTwoNumbers), a(a_), b(b_) {}
};

// Params for the call example_copy_string_to_heap()
class alignas(__BIGGEST_ALIGNMENT__) CExampleCopyStringToHeapCallCompartmentData : public CCompartmentData
{
public:
    const char* str;

public:
    CExampleCopyStringToHeapCallCompartmentData(
        const char* str_
    ) : CCompartmentData(CompCall_callExampleCopyStringToHeap), str(str_) {}
};

// Params for the call example_print_heap_string_and_free()
class alignas(__BIGGEST_ALIGNMENT__) CExamplePrintHeapStringAndFreeCallCompartmentData : public CCompartmentData
{
public:
    char* str;
    int16_t  chars_to_print;

public:
    CExamplePrintHeapStringAndFreeCallCompartmentData(
        char* str_,
        int16_t  chars_to_print_
    ) : CCompartmentData(CompCall_callExamplePrintHeapStringAndFree), str(str_), chars_to_print(chars_to_print_) {}
};

// Params for the call example_dump_struct()
class alignas(__BIGGEST_ALIGNMENT__) CExampleDumpStructCallCompartmentData : public CCompartmentData
{
public:
    const struct example_struct* data;

public:
    CExampleDumpStructCallCompartmentData(
        const struct example_struct* data_
    ) : CCompartmentData(CompCall_callExampleDumpStruct), data(data_) {}
};

// Params for the call example_set_compartment_debug_level()
class alignas(__BIGGEST_ALIGNMENT__) CExampleSetCompartmentDebugLevelCallCompartmentData : public CCompartmentData
{
public:
    int32_t debug_level;

public:
    CExampleSetCompartmentDebugLevelCallCompartmentData(
        int32_t debug_level_
    ) : CCompartmentData(CompCall_callExampleSetCompartmentDebugLevel), debug_level(debug_level_) {}
};

#endif /* _COMPARTMENT_DATA_H__ */

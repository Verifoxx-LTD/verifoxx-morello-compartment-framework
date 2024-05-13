/*
 * Copyright (C) 2024 Verifoxx Limited
 * Capability Manager main program which loads compartment library and calls example API functions.
 */

// We use GLIBC internals...
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// C includes
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <cheriintrin.h>

// C++ includes
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <vector>

// CapMgr Includes
#include "CCapability.h"
#include "CCompartment.h"
#include "CCompartmentData.h"
#include "CCompartmentLibs.h"
#include "CCapMgrLogger.h"
#include "CCompartmentApiProxy.h"

// The example API we will call proxy functions for
#include "example_comp_api.h"

using namespace CapMgr;

/* Simple logging */
void set_capmgr_log_level(uint32_t log_level)
{
    if (log_level > (uint32_t)VERBOSE)
        log_level = (uint32_t)VERBOSE;

    Log::Level() = (TLogLevel)log_level;
}


/* Capability Manager Support: Load compartment library and patch relocation symbols */
static bool lib_load_and_fix(const std::string& libname, CCompartmentLibs*& plibs, bool dump_tables = false)
{
    auto rwcap{ Capability(getauxptr(AT_CHERI_EXEC_RW_CAP)) };
    auto fixup_cap{ Capability(getauxptr(AT_CHERI_EXEC_RW_CAP)) };

#if CAPMGR_BUILT_STATIC_ENABLE
    bool load_new = false;  // For static build, the cap mgr has no linkmap so cannot load a new one
#else
    bool load_new = true;
#endif
    plibs = new CCompartmentLibs{ libname, rwcap, fixup_cap, load_new };

    if (dump_tables)
    {
        L_(ALWAYS) << "Dump libs phdrs: " << *plibs;
        L_(ALWAYS) << "Dump reloc tables: " << plibs->DumpRelocTables();
    }

    L_(DEBUG) << "Do capability relocation fixups...";
    return plibs->DoAllLibCapFixups();
}

/* Capability Manager Support: Fixup relocation symbols ahead of program exit */
// @todo: make it part of CCompartmentLibs destructor?
static bool lib_restore_and_end(CCompartmentLibs* plibs)
{
#if CAPMGR_BUILT_STATIC_ENABLE
    (void)plibs;    // Silence the warning
    L_(VERBOSE) << "No action to revert fixups needed for static build";
    return true;
#else

    L_(DEBUG) << "Revert capability relocation fixups...";

    auto result = plibs->DoAllLibCapFixups(false);

    L_(DEBUG) << "Delete the libs and dlclose()";
    delete plibs;
    return result;
#endif
}

static int print_help(const char *exe_name)
{
    printf("Usage: %s [-options]\n", exe_name);
    printf("options:\n");

    printf("  --comp-lib=<lib>       Load Shared object (.so) containing code to run in compartment\n");
    printf("                         Defaults to .\\libcompartment.so\n");

    printf("  -v=n                   Set log verbose level (0 to 4, default is 2) larger\n"
        "                           level gives higher verbosity.\n");
    printf("  --dump_tables          Dump relocation tables to stdout\n");
    return 1;
}

// Helper for example structure stream
std::ostream& operator<<(std::ostream& ostream, const struct example_struct struct_data)
{
    ostream << "{ i=" << struct_data.i << " : b=" << std::boolalpha
        << struct_data.b << " : c=" << struct_data.c << "}";
    return ostream;
}


int main(int argc, char* argv[])
{
    int32_t ret = -1;
    bool dump_relocation_tables = false;
    int32_t log_verbose_level = (uint32_t)WARNING;

    std::string comp_lib{"./libcompartment.so"};    // Test shared object library

    /* Process options. */
    for (argc--, argv++; argc > 0 && argv[0][0] == '-'; argc--, argv++) {
        
        if (!strncmp(argv[0], "--comp-lib=", 11)) {
            if (argv[0][11] == '\0')
                return print_help(argv[0]);
            comp_lib = argv[0] + 11;
        }
        else if (!strncmp(argv[0], "-v=", 3)) {
            log_verbose_level = atoi(argv[0] + 3);

            // Log level 
            if (log_verbose_level < 0 || log_verbose_level > (int32_t)VERBOSE)
                return print_help(argv[0]);
        }
        else if (!strncmp(argv[0], "--dump_tables", 13)) {
            dump_relocation_tables = true;
        }
        else
            return print_help(argv[0]);
    }

    if (argc != 0)
        return print_help(argv[0]);

    
    set_capmgr_log_level(log_verbose_level);

    L_(ALWAYS) << "Running " << argv[0] << " Examples..." << std::endl;

    /* Load compartment library and resolve symbol relocations
     * Then create proxy object for compartment calls
     */
    CCompartmentLibs* plibs = nullptr;
    if (!lib_load_and_fix(comp_lib, plibs, dump_relocation_tables))
    {
        L_(ERROR) << "Compartment Libary " << comp_lib << " is not valid or could not be found" << std::endl;
        return -1;
    }

    // Create the proxy - note the compartment sizes are defined in CCompartment.h
    CCompartmentApiProxy proxy(plibs, CCompartment::CompartmentId::kCompartmentExampleId, CALL_FUNC_STACK_SIZE, CALL_FUNC_SEAL_ID);

    L_(ALWAYS) << "Set compartment debug level using capability manager's log level" << std::endl;
    auto log_result = proxy.example_set_compartment_debug_level(log_verbose_level);
    L_(ALWAYS) << "Result of example_set_compartment_debug_level(" << log_verbose_level << ") = "
        << std::boolalpha << log_result << std::endl;

    int32_t test1 = 3;
    int32_t test2 = 8;
    L_(ALWAYS) << "Perform example_add_two_numbers()" << std::endl;
    auto result = proxy.example_add_two_numbers(test1, test2);
    L_(ALWAYS) << "Result of example_add_two_numbers(" << test1 << ", " << test2 << ") = " << result << std::endl;

    std::string test_str{ "This is a test" };
    L_(ALWAYS) << "Perform example_copy_string_to_heap()" << std::endl;
    auto buff = proxy.example_copy_string_to_heap(test_str.c_str());
    L_(ALWAYS) << "Result of example_copy_string_to_heap(\"" << test_str << "\") = " << buff << std::endl;

    int32_t num = 7;
    L_(ALWAYS) << "Perform example_print_heap_string_and_free()" << std::endl;
    bool success = proxy.example_print_heap_string_and_free(buff, num);
    L_(ALWAYS) << "Result of example_print_heap_string_and_free(<buffer>, " << num << ") = "
        << std::boolalpha << success << std::endl;

    struct example_struct test_struct { 99, false, '!' };
    L_(ALWAYS) << "Perform example_dump_struct("<< test_struct << ")" << std::endl;
    proxy.example_dump_struct(&test_struct);
    L_(ALWAYS) << "example_dump_struct() completed" << std::endl;

    L_(ALWAYS) << "*EXAMPLE ENDS*" << std::endl;
    ret = 0;

    /* Cleanup the relocation symbols*/
    if (!lib_restore_and_end(plibs))
    {
        L_(ERROR) << "Error unloading compartment library " << comp_lib << std::endl;
        ret = -1;
    }

    L_(DEBUG) << "Capability Manager exits with return code: " << ret << "!";
    return ret;
}

// Compartment entry in resticted

#include <iostream>
#include <cstdlib>
#include <memory>

#include "comp_common_defs.h"
#include "comp_caller.h"
#include "CCompartmentData.h"

#include "example_comp_api.h"
#include "service_call_proxy.h"
#include "compartment_basic_logger.h"

// Global CServiceCallProxy ptr which is set to the single CServiceCallProxy on each compartment call
static std::unique_ptr<CServiceCallProxy> g_service_call_proxy;

// Accessor for user classes
CServiceCallProxy *CServiceCallProxy::GetInstance()
{
    return g_service_call_proxy.get();
}

// Call function looks up type of derived class data and then static_cast() to resolve
static uintptr_t CallFunction(CCompartmentData* p)
{
    uintptr_t result{ 0 };

    switch (p->comp_call_type)
    {
        case CompCall_callExampleAddTwoNumbers:
        {
            auto p_d = static_cast<CExampleAddTwoNumbersCallCompartmentData*>(p);
            auto real_fp = reinterpret_cast<FnPtr_example_add_two_numbers>(p_d->fp);

            LOG_DEBUG("Calling example_add_two_numbers()");
            result = (uintptr_t)real_fp(p_d->a, p_d->b);
        }
        break;

        case CompCall_callExampleCopyStringToHeap:
        {
            auto p_d = static_cast<CExampleCopyStringToHeapCallCompartmentData*>(p);
            auto real_fp = reinterpret_cast<FnPtr_example_copy_string_to_heap>(p_d->fp);

            LOG_DEBUG("Calling example_copy_string_to_heap()");
            result = (uintptr_t)real_fp(p_d->str);
        }
        break;

        case CompCall_callExamplePrintHeapStringAndFree:
        {
            auto p_d = static_cast<CExamplePrintHeapStringAndFreeCallCompartmentData*>(p);
            auto real_fp = reinterpret_cast<FnPtr_example_print_heap_string_and_free>(p_d->fp);

            LOG_DEBUG("Calling example_print_heap_string_and_free()");
            result = (uintptr_t)real_fp(p_d->str, p_d->chars_to_print);
        }
        break;

        case CompCall_callExampleDumpStruct:
        {
            auto p_d = static_cast<CExampleDumpStructCallCompartmentData*>(p);
            auto real_fp = reinterpret_cast<FnPtr_example_dump_struct>(p_d->fp);

            LOG_DEBUG("Calling example_dump_struct()");
            real_fp(p_d->data);
        }
        break;

        case CompCall_callExampleSetCompartmentDebugLevel:
        {
            auto p_d = static_cast<CExampleSetCompartmentDebugLevelCallCompartmentData*>(p);
            auto real_fp = reinterpret_cast<FnPtr_example_set_compartment_debug_level>(p_d->fp);

            LOG_DEBUG("Calling example_set_compartment_debug_level");
            result = (uintptr_t)real_fp(p_d->debug_level);
        }
        break;

        default:
        {
            LOG_ERROR("Failed to call Compartment function - unsupported function");
        }
        break;
    }
    LOG_DEBUG("\tCompartment function call returned");
    return result;
}

// CompartmentUnwrap: Given pointer to the data table
extern "C" void CompartmentUnwrap(void* comp_data_object)
{
    LOG_DEBUG("--> COMPARTMENT ENTRY -->");

    CCompartmentData *comp_fn_data = reinterpret_cast<CCompartmentData*>(comp_data_object);

    // Create the service call proxy
    g_service_call_proxy = std::make_unique<CServiceCallProxy>(comp_fn_data);

    LOG_VERBOSE("Dump capabilities from capability manager:\n"
        "\t\tCapMgr return FP=%#p\n"
        "\t\tCompartment Fn to call FP=%#p\n"
        "\t\tService Callback Entry FP=%#p\n"
        "\t\tService Callback Handler FP=%#p\n"
        "\t\tService Callback FP LUT=%#p\n"
        "\t\tSealer Capability=%#p",
        comp_fn_data->comp_exit_fp,
        comp_fn_data->fp,
        comp_fn_data->service_callback_entry_fp,
        comp_fn_data->capmgr_service_fp,
        (void*)comp_fn_data->service_func_table,
        comp_fn_data->sealer_cap);

    // Get compartment data to call implementation specific function
    uintptr_t retval = CallFunction(comp_fn_data);

    g_service_call_proxy.release(); // Cleanup, proxy no longer needed

    // Call compartment return passing our exit function pointer
    CompartmentReturn(comp_fn_data->comp_exit_fp, retval);
}

extern "C" void CompartmentReturn(CompExitAsmFnPtr fp, uintptr_t return_arg)
{
    // Compartment calls fp to return, pass back return_arg as argument
    LOG_DEBUG("<-- COMPARTMENT EXIT <--");

    fp(return_arg);
    __builtin_unreachable();
}

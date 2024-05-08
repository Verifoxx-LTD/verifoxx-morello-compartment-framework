// CCompartmentApiProxy: Proxy for API calls which are implemented in the compartment

#ifndef _CCOMPARTMENT_API_PROXY_H__
#define _CCOMPARTMENT_API_PROXY_H__

#include <string>
#include <memory>

#include "example_comp_api.h"
#include "CCompartment.h"
#include "comp_common_defs.h"
#include "CCompartmentData.h"

class CCompartmentApiProxy
{
    CCompartment m_compartment;

public:
    CCompartmentApiProxy(const CCompartmentLibs* comp_libs, CCompartment::CompartmentId id, uint32_t stack_size, uint32_t seal_id)
        : m_compartment(comp_libs, id, stack_size, seal_id) {}

    template <typename T, typename... Args>
    uintptr_t CallApiFn(const std::string& fn_name, Args&&... args)
    {
        return m_compartment.CallCompartmentFunction(fn_name,
            std::make_shared<T>(std::forward<Args>(args)...)
        );
    }


    /* Add an entry for each API function.  Could be more automated via C Macros */
    template <typename... Args>
    int32_t example_add_two_numbers(Args&&... args)
    {
        return (int32_t)CallApiFn<CExampleAddTwoNumbersCallCompartmentData>(__func__, std::forward<Args>(args)...);
    }

    template <typename... Args>
    char *example_copy_string_to_heap(Args&&... args)
    {
        return (char *)CallApiFn<CExampleCopyStringToHeapCallCompartmentData>(__func__, std::forward<Args>(args)...);
    }

    template <typename... Args>
    bool example_print_heap_string_and_free(Args&&... args)
    {
        return (bool)CallApiFn<CExamplePrintHeapStringAndFreeCallCompartmentData>(__func__, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void example_dump_struct(Args&&... args)
    {
        CallApiFn<CExampleDumpStructCallCompartmentData>(__func__, std::forward<Args>(args)...);
    }

    template <typename... Args>
    bool example_set_compartment_debug_level(Args&&... args)
    {
        return (bool)CallApiFn<CExampleSetCompartmentDebugLevelCallCompartmentData>(__func__, std::forward<Args>(args)...);
    }
};

#endif /* _CCOMPARTMENT_API_PROXY_H__ */


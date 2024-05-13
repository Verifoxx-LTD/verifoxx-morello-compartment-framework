 /* Copyright (C) 2024 Verifoxx Limited
  * example_capmgr_service_api_impl: Capability manager service implementation of the
  * example service callback functions
  */

#include "example_capmgr_service_api.h"

#include <cstdlib>
#include <cstring>
#include <cheriintrin.h>
#include "CCapMgrLogger.h"

using namespace CapMgr;

void* cheri_malloc(size_t size)
{
    L_(DEBUG) << "System malloc: size=" << size;
    void* ptr = std::malloc(size);

    if (ptr)
    {
        std::memset(ptr, 0, size);  // Slight extension, we clear the memory buffer after alloc
    }

    return cheri_perms_clear(ptr, ARM_CAP_PERMISSION_EXECUTIVE);    // Set suitable for restricted
}

void  cheri_free(void* ptr)
{
    L_(DEBUG) << "System free memory";
    std::free(ptr);
}

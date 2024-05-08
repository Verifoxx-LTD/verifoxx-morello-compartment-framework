/* example_capmgr_service_api: Example / Demo API implemented by a capability manager, to provide services for the compartment
* The compartment, running in restricted, cannot access system or native calls.
* To allow this the capability manager provides "services" and presents an API which can be used by compartment library functions
*/

#ifndef _EXAMPLE_CAPMGR_SERVICE_API__
#define _EXAMPLE_CAPMGR_SERVICE_API__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    void* cheri_malloc(size_t sz_bytes);
    void  cheri_free(void* ptr);

    //@todo: Add printf() support

#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_CAPMGR_SERVICE_API__ */

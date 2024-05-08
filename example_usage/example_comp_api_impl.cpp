/* example_comp_api_impl: Library functions which run in the compartment, called from capability manager
* These may make service callbacks for system calls inside the capability manager
*/

#include <cstdlib>
#include <cstring>
#include <cheriintrin.h>

#include "example_comp_api.h"
#include "compartment_basic_logger.h"
#include "service_call_proxy.h"

using namespace std;

extern "C" int32_t example_add_two_numbers(int32_t a, int32_t b)
{
    LOG_VERBOSE("example_add_two_numbers(%d, %d)", a, b);
    int32_t c = a + b;
    LOG_DEBUG("example_add_two_numbers: result = %d", c);
    LOG_VERBOSE("example_add_two_numbers: finished");
    return c;
}

extern "C" char* example_copy_string_to_heap(const char* str)
{
    LOG_VERBOSE("example_copy_string_to_heap(\"%s\")", str);

    // Service callback for malloc()
    auto sz = strlen(str);
    LOG_DEBUG("example_copy_string_to_heap: Service callback to allocate %d bytes", sz);

    char* mem_buff = static_cast<char*>(CServiceCallProxy::GetInstance()->cheri_malloc(sz + 1));    // Function will prefill buffer with zeros

    if (mem_buff)
    {
        LOG_DEBUG("example_copy_string_to_heap: Allocated memory ok!");
        strncpy(mem_buff, str, sz);
    }
    LOG_VERBOSE("example_copy_string_to_heap: finished");
    return mem_buff;
}

extern "C" bool example_print_heap_string_and_free(char* str, int16_t chars_to_print)
{
    LOG_VERBOSE("example_print_heap_string_and_free(\"%s\", %d)", str, chars_to_print);

    auto sz = strlen(str);
    LOG_FATAL("Print String:");
    
    auto posn = chars_to_print < sz ? chars_to_print : sz;
    auto c = str[posn];
    str[posn] = '\0';
    printf("%s\n", str);
    str[posn] = c;
    LOG_DEBUG("Printed first %d characters of \"%s\"", chars_to_print, str);

    LOG_DEBUG("example_print_heap_string_and_free: Service callback to free heap memory");

    CServiceCallProxy::GetInstance()->cheri_free(str);

    LOG_VERBOSE("example_print_heap_string_and_free: finished");
    return true;
}

extern "C" void example_dump_struct(const struct example_struct* data)
{
    LOG_VERBOSE("example_dump_struct(<data>)");
    LOG_FATAL("Dump:");
    printf("Int = %d : Bool = %s : Char = \"%c\"\n", data->i, data->b ? "true" : "false", data->c);
    LOG_VERBOSE("example_dump_struct: finished");
}

extern "C" bool example_set_compartment_debug_level(int32_t debug_level)
{
    LOG_VERBOSE("example_set_compartment_debug_level(%d)", debug_level);
    if (debug_level < (int32_t)LOG_LEVEL_FATAL || debug_level >(int32_t)LOG_LEVEL_VERBOSE)
    {
        LOG_ERROR("Log level out of range");
        return false;
    }
    set_log_verbosity_level(debug_level);
    LOG_DEBUG("Log level updated");
    return true;
}

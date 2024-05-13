/* Copyright (C) 2024 Verifoxx Limited
 * Very basic C logger for the compartment - Example code.
 * IMPORTANT: As printf() is a system call, this should involve a callback to the capability manager.
 * Currently, system calls are still available from restricted therefore this still works (for now!)
 */

#ifndef _COMPARTMENT_BASIC_LOGGER_H__
#define _COMPARTMENT_BASIC_LOGGER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum {
        LOG_LEVEL_FATAL = 0,
        LOG_LEVEL_ERROR = 1,
        LOG_LEVEL_WARNING = 2,
        LOG_LEVEL_DEBUG = 3,
        LOG_LEVEL_VERBOSE = 4
    } LogLevel;

    void set_log_verbosity_level(int32_t level);

    void log_msg(LogLevel log_level, const char* fmt, ...);
    
    #define LOG_FATAL(...) log_msg(LOG_LEVEL_FATAL, __VA_ARGS__)
    #define LOG_ERROR(...) log_msg(LOG_LEVEL_ERROR, __VA_ARGS__)
    #define LOG_WARNING(...) log_msg(LOG_LEVEL_WARNING, __VA_ARGS__)
    #define LOG_VERBOSE(...) log_msg(LOG_LEVEL_VERBOSE, __VA_ARGS__)
    #define LOG_DEBUG(...) log_msg(LOG_LEVEL_DEBUG, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* _COMPARTMENT_BASIC_LOGGER_H__ */


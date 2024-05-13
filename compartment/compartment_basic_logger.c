// Copyright (C) 2024 Verifoxx Limited
// Implement a compartment_basic_logger.  Example code.

#include <stdio.h>
#include <stdarg.h>


#include "compartment_basic_logger.h"

static int32_t g_log_level = LOG_LEVEL_VERBOSE;

void set_log_verbosity_level(int32_t level)
{
    g_log_level = level;
}

void log_msg(LogLevel log_level,const char* fmt, ...)
{

    if ((int32_t)log_level > g_log_level)
        return;

    printf("\tCOMPARTMENT-LOG => ");

    va_list args_list;
    va_start(args_list, fmt);
    vprintf(fmt, args_list);
    va_end(args_list);

    printf("\n");
}

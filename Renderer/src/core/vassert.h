#pragma once

#include "defines.h"

#ifdef VASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap() // Triggers breakpoint
#endif

VAPI void report_assertion_failure(const char* expression, const char* msg, const char* file, i32 line);

#define VASSERT(expr)                                               \
    {                                                               \
        if (expr)                                                   \
        { /* Do nothing */ }                                        \
        else                                                        \
        {                                                           \
            report_assertion_failure(#expr,"",__FILE__,__LINE__);  \
            debugBreak();                                           \
        }                                                           \
    }                                                               

#define VASSERT_MSG(expr,message)                                           \
    {                                                                       \
        if (expr)                                                           \
        { /* Do nothing */ }                                                \
        else                                                                \
        {                                                                   \
            report_assertion_failure(#expr,message,__FILE__,__LINE__);     \
            debugBreak();                                                   \
        }                                                                   \
    }                                                                         

#ifdef VKR_DEBUG
#define VASSERT_DEBUG(expr)                                             \
    {                                                                   \
        if (expr)                                                       \
        { /* Do nothing */ }                                            \
        else                                                            \
        {                                                               \
            report_assertion_failure(#expr, "", __FILE__, __LINE__);   \
            debugBreak();                                               \
        }                                                               \
    }                                                                   
#else
#define VASSERT_DEBUG(expr) // Does nothing
#endif

#else
#define VASSERT(expr) // Does nothing if assertions are not enabled
#define VASSERT_MSG(expr,msg) 
#define VASSERT_DEBUG(expr)
#endif

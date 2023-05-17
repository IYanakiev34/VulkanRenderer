#pragma once

#include "defines.h"

/*
* These types of logs should be switchable. Only error nad fatal
* should be logged in release and distribution builds.
*/
#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_TRACE_ENABLED 1
#define LOG_DEBUG_ENABLED 1

#if VKR_RELEASE == 1 || VKR_DIST == 1
#define LOG_DEBUG_ENABLE 0
#define LOG_TRACE_ENABLE 0
#endif


typedef enum log_level {
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} log_level;


b8 intialize_logging();
void shutdown_logging();


VAPI void log_output(log_level level , const char* msg, ...);

// We always need fatal logging
#define VFATAL(message,...) log_output(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);

// Make sure error logging is enabled
#ifndef VERROR
#define VERROR(message,...) log_output(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#endif

// If wanr level is enabled define function call
#if LOG_WARN_ENABLED == 1
#define VWARN(message,...) log_output(LOG_LEVEL_WARN,message, ##__VA_ARGS__);
#elif
// Does nothing if warn level not enabled
#define VWARN(message,...)
#endif

// If debug level is enabled define function call
#if LOG_DEBUG_ENABLED == 1
#define VDEBUG(message,...) log_output(LOG_LEVEL_DEBUG,message, ##__VA_ARGS__);
#elif
// Does nothing if debug level not enabled
#define VDEBUG(message,...)
#endif

#if LOG_INFO_ENABLED == 1
#define VINFO(message,...) log_output(LOG_LEVEL_INFO,message, ##__VA_ARGS__);
#elif
// Does nothing if wanr level not enabled
#define VINFO(message,...)
#endif

// If Trace level is enabled define function call
#if LOG_TRACE_ENABLED == 1
#define VTRACE(message,...) log_output(LOG_LEVEL_TRACE,message, ##__VA_ARGS__);
#elif
// Does nothing if trace level not enabled
#define VTRACE(message,...)
#endif


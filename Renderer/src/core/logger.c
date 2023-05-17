#include "logger.h"
#include "vassert.h"
#include "platform/platform.h"

// TODO: temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

//////////////////// From vassert.h /////////////////////////////////
void report_assertion_failure(const char* expression, const char* msg, const char* file, i32 line)
{
    log_output(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s', in file: %s, line: %i", expression, msg, file, line);
}


b8 intialize_logging() {
    // TODO: Create log file
}

void shutdown_logging() {
    // TODO: cleanup logging queued entries
}

void log_output(log_level level, const char* msg, ...) {
    const char* level_strings[6] = { "[TRACE]:","[DEBUG]:","[INFO]:","[WARN]:","[ERROR]:","[FATAL]:" };
    b8 is_error = level > LOG_LEVEL_WARN; // Is ERROR or FATAL

    // Should not do that technically limit 500 maybe not be enough
    u8 int_buffer[5000];
    memset(int_buffer, 0, sizeof(int_buffer));

    // Format original message
    va_list arg_ptr; // Argument pointer
    va_start(arg_ptr, msg); // Get the start
    vsnprintf(int_buffer, 5000, msg, arg_ptr); // Format parameters into the buffer
    va_end(arg_ptr); // Bring pointer to end
    
    u8 out_buffer[5000];
    memset(out_buffer, 0, sizeof(out_buffer));
    sprintf(out_buffer, "%s%s\n", level_strings[level], int_buffer);
    
    // Output colored message based on level of severity
    if (is_error)
        platform_console_write_error(out_buffer, level);
    else
        platform_console_write(out_buffer, level);
}
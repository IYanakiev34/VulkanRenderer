#pragma once
#include "defines.h"

typedef struct clock {
    f64 start_time;
    f64 elapsed_time; // In seconds
} clock;

// Update the elapsed time of the clock
void clock_update(clock* clock);

// Starts the passed clock and resets elapsed time
void clock_start(clock* clock);

// Stop the clock. Meaning do not reset elapsed time
void clock_stop(clock* clock);
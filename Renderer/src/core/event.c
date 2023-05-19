#include "event.h"
#include "vmemory.h"
#include "logger.h"
#include "containers/darray.h"

// Store listener information with callback function
typedef struct registered_event {
    void* listener;
    PFN_on_event cb;
} registered_event;

// Each event code can have multiple listerners
typedef struct event_code_entry {
    registered_event* events;
} event_code_entry;

#define MAX_MESSAGE_CODE 16384

// State structure
typedef struct event_system_state {
    // Lookup table for event codes
    event_code_entry registered[MAX_MESSAGE_CODE];
} event_system_state;

static b8 initialized = FALSE;
static event_system_state state;

b8 event_initialize() {
    if (initialized)
    {
        VERROR("Event system is already initialized");
        return FALSE;
    }

    vzero_memory(&state, sizeof(state));
    initialized = TRUE;
    return TRUE;
}

void event_shutdown() {
    // Free all event arrays
    for (u16 idx = 0; idx != MAX_MESSAGE_CODE; ++idx) {
        if (state.registered[idx].events != 0) {
            darray_destroy(state.registered[idx].events);
            state.registered[idx].events = 0;
        }
    }
}

b8 event_register(u16 code, void* listener_inst, PFN_on_event on_event_cb) {
    if (!initialized) {
        return FALSE;
    }

    if (state.registered[code].events == 0) {
        state.registered[code].events = darray_create(registered_event);
    }

    u64 registered_count = darray_length(state.registered[code].events);
    for (u64 idx = 0; idx != registered_count; ++idx) {
        if (state.registered[code].events[idx].listener == listener_inst) {
            VWARN("Listener has already been registered for this event! Event code: %i", code);
            return FALSE;
        }
    }

    // If no duplicate was found push the event to the list of events for this code
    registered_event event;
    event.listener = listener_inst;
    event.cb = on_event_cb;
    darray_push(state.registered[code].events, event);

    return TRUE;
}

b8 event_unregister(u16 code, void* listener_inst, PFN_on_event on_event_cb) {
    if (!initialized) {
        return FALSE;
    }

    if (state.registered[code].events == 0) {
        VERROR("There are no events with that code registered! Code: %i", code);
        return FALSE;
    }

    u64 registered_count = darray_length(state.registered[code].events);
    for (u64 idx = 0; idx != registered_count; ++idx) {
        registered_event e = state.registered[code].events[idx];
        if (e.listener == listener_inst && e.cb == on_event_cb) {
            registered_event popped_ev;
            darray_pop_at(state.registered[code].events, idx, &popped_ev);
            return TRUE;
        }
    }

    VWARN("Could not unregister event since it was not registered.")
    return FALSE;
}

b8 event_fire(u16 code, void* sender, event_context data) {
    if (!initialized) {
        return FALSE;
    }

    if (state.registered[code].events == 0) {
        VWARN("Cannot fire event since it is not registered! Event code: %i", code);
        return FALSE;
    }

    u64 registered_count = darray_length(state.registered[code].events);

    for (u64 idx = 0; idx != registered_count; ++idx) {
        registered_event e = state.registered[code].events[idx];
        if (e.cb(code, sender, e.listener, data)) {
            // If event handled do not dispatch further
            return TRUE;
        }
    }

    // Not found
    return FALSE;
}
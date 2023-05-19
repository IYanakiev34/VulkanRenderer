#pragma once
#include "defines.h"

/*
* The data that each event will hold.
* Depending on event type different parts of the union
* will be filed.
*/
typedef struct event_context {
    union {
        i64 i64[2];
        u64 u64[2];
        f64 f64[2];

        i32 i32[4];
        u32 u32[4];
        f32 f32[4];

        i16 i16[8];
        u16 u16[8];

        i8 i8[16];
        u8 u8[16];

        char c[16];
    } data;
} event_context;

// Return true if handled
typedef b8(*PFN_on_event)(u16 code, void* sender, void* listener_inst, event_context data);

/*
* Initialize the event system. Set the state of the
* system to 0.
* 
* @return b8 - TRUE if successful, FALSE is not or system has already been initialized and called again
*/
b8 event_initialize();

/**
* Deallocate any memory the system currently uses and handle
* the rest of the events (if stored in a queue)
*/
void event_shutdown();

/**
* Registers a listener callback to an event code.
* 
* @param code - The code of the event (in the enumeration for events)
* @param listener_inst - The entity which will listen for the event
* @param on_event_cb - The callback function fired once the event is recieved
* 
* @return b8 - TRUE if successful, FALSE if (1. listener alrerady registered, 2. system not initialized)
*/
VAPI b8 event_register(u16 code, void* listener_inst, PFN_on_event on_event_cb);

/**
* Unregisters an entity from an event. The entity will no longer be notified
* once the event occurs.
* 
* @param code - The code of the event (in the enumeration for events)
* @param listener_inst - The entity which had listened for the event
* @param on_event_cb - The callback function of the listener
* 
* @return b8 - TRUE if we could unegister listener, FALSE if (1. listener was not registered in the first place, 2. event with this code does not exist)
*/
VAPI b8 event_unregister(u16 code, void* listener_inst, PFN_on_event on_event_cb);

/**
* Fires an event with a specific code. All listeners will get notified.
* If the event gets handled along the way no more listeners will be notified,
* 
* @param code - The code of the event (in the enumeration for events)
* @param sender - The entity which sends the event
* @param data - The data for the event (could be mouse button, position, etc). Data layout is defined in enumeration for events.
* 
* @return b8 - TRUE if the event was handled, FALSE is not or does not exist
*/
VAPI b8 event_fire(u16 code, void* sender, event_context data);

// Engine only code from 0 to 255, User codes should start from 256
typedef enum system_event_code {
    // Called when application quits
    EVENT_CODE_APPLICATION_QUIT = 0x01,

    /*
    * Fill in the data.u16[0] = key when pressed
    */
    EVENT_CODE_KEY_PRESSED = 0x02,

    /*
    * Fill in the data.u16[0] = key when released
    */
    EVENT_CODE_KEY_RELEASED = 0x03,
    
    /*
    * Fill in the data.u16[0] = mouse button pressed
    */
    EVENT_CODE_BUTTON_PRESSED = 0x04,

    /*
    * Fill in the data.u16[0] = mouse button released
    */
    EVENT_CODE_BUTTON_RELEASED = 0x05,

    /*
    * Fill in the data.i16[0] = x coordinate
    * Fill in the data.i16[1] = y coordinate
    */
    EVENT_CODE_MOUSE_MOVED = 0x06,

    /*
    * Fill in the data.i8[0] = z_delta
    */
    EVENT_CODE_MOUSE_WHEEL = 0x07,
    
    // Called when window gets resized
    EVENT_CODE_RESIZED = 0x08,

    // Max code for engine events
    MAX_EVENT_CODE = 0xFF,
} system_event_code;



#pragma once
#include "defines.h"

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
}event_context;

// Return true if handled
typedef b8(*PFN_on_event)(u16 code, void* sender, void* listener_inst, event_context data);

b8 event_initialize();
void event_shutdown();

VAPI b8 event_register(u16 code, void* listener_inst, PFN_on_event on_event_cb);

VAPI b8 event_unregister(u16 code, void* listener_inst, PFN_on_event on_event_cb);

VAPI b8 event_fire(u16 code, void* sender, event_context data);

// Engine only code from 0 to 255, User codes should start from 256
typedef enum system_event_code {
    // Called when application quits
    EVENT_CODE_APPLICATION_QUIT = 0x01,

    // Called when a key on keyboard is pressed
    EVENT_CODE_KEY_PRESSED = 0x02,

    // Called when key on keyboard is relaesed
    EVENT_CODE_KEY_RELEASED = 0x03,
    
    // Called when button on mouse is pressed
    EVENT_CODE_BUTTON_PRESSED = 0x04,

    // Called when button on mouse is released
    EVENT_CODE_BUTTON_RELEASED = 0x05,

    // Called when the mouse moved
    EVENT_CODE_MOUSE_MOVED = 0x06,

    // Called when the mouse wheel moved
    EVENT_CODE_MOUSE_WHEEL = 0x07,
    
    // Called when window gets resized
    EVENT_CODE_RESIZED = 0x08,

    // Max code for engine events
    MAX_EVENT_CODE = 0xFF,
};



#include "input.h"
#include "logger.h"
#include "vmemory.h"
#include "event.h"

typedef struct keyboard_state {
    b8 keys[256];
} keyboard_state;

typedef struct mouse_state {
    i16 x;
    i16 y;
    b8 buttons[MB_MAX_BUTTONS];
} mouse_state;

typedef struct input_state {
    keyboard_state keyboard_current;
    keyboard_state keyboard_previous;
    mouse_state mouse_current;
    mouse_state mouse_previous;
} input_state;

static b8 initialized = FALSE;
static input_state state;


b8 input_initialize() {
    if (initialized) {
        VERROR("Input system is already initialized");
        return FALSE;
    }

    vzero_memory(&state, sizeof(state));
    initialized = TRUE;
    VINFO("Input system has been initialized");
    return TRUE;
}

void input_shutdown() {
    // TODO: shutdown routines when needed
    initialized = FALSE;
}

void input_update(f64 delta_time) {
    if (!initialized) {
        return;
    }

    // Copy current states to previous states
    vcopy_memory(&state.keyboard_previous, &state.keyboard_current, sizeof(keyboard_state));
    vcopy_memory(&state.mouse_previous, &state.mouse_current, sizeof(mouse_state));
}

b8 input_is_key_down(keys key) {
    if (!initialized) {
        return FALSE;
    }

    return state.keyboard_current.keys[key] == TRUE;
}

b8 input_is_key_up(keys key) {
    if (!initialized) {
        return FALSE;
    }

    return state.keyboard_current.keys[key] == FALSE;
}

b8 input_was_key_down(keys key) {
    if (!initialized) {
        return FALSE;
    }

    return state.keyboard_previous.keys[key] == TRUE;
}

b8 input_was_key_up(keys key) {
    if (!initialized) {
        return FALSE;
    }

    return state.keyboard_previous.keys[key] == FALSE;
}

void input_process_key(keys key, b8 pressed) {
    if (state.keyboard_current.keys[key] != pressed) {
        state.keyboard_current.keys[key] = pressed;

        // Fire event for the button process immediately currently
        event_context event;
        event.data.u16[0] = key;
        event_fire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, event);
    }
}

b8 input_is_button_down(mouse_buttons button) {
    if (!initialized) {
        return FALSE;
    }

    return state.mouse_current.buttons[button] == TRUE;
}

b8 input_is_button_up(mouse_buttons button) {
    if (!initialized) {
        return FALSE;
    }

    return state.mouse_current.buttons[button] == FALSE;
}

b8 input_was_button_down(mouse_buttons button) {
    if (!initialized) {
        return FALSE;
    }

    return state.mouse_previous.buttons[button] == TRUE;
}

b8 input_was_button_up(mouse_buttons button) {
    if (!initialized) {
        return FALSE;
    }

    return state.mouse_previous.buttons[button] == FALSE;
}

void input_get_mouse_position(i32* x, i32* y) {
    if (!initialized) {
        return;
    }

    *x = state.mouse_current.x;
    *y = state.mouse_current.y;
}

void input_get_previous_mouse_position(i32* x, i32* y) {
    if (!initialized) {
        return;
    }

    *x = state.mouse_previous.x;
    *y = state.mouse_previous.y;
}

void input_process_button(mouse_buttons button, b8 pressed) {
    if (state.mouse_current.buttons[button] != pressed) {
        state.mouse_current.buttons[button] = pressed;

        event_context event;
        event.data.u16[0] = button;
        event_fire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, 0, event);
    }
}

void input_process_mouse_move(i16 x, i16 y) {
    i16 curr_x = state.mouse_current.x;
    i16 curr_y = state.mouse_current.y;
    if (curr_x != x || curr_y != y) {
        state.mouse_current.x = x;
        state.mouse_current.y = y;

        event_context event;
        event.data.i16[0] = x;
        event.data.i16[1] = y;

        event_fire(EVENT_CODE_MOUSE_MOVED, 0, event);
    }
}

void input_process_mouse_wheel(i8 z_delta) {
    // No state to update
    event_context event;
    event.data.i8[0] = z_delta;
    event_fire(EVENT_CODE_MOUSE_WHEEL, 0, event);
}

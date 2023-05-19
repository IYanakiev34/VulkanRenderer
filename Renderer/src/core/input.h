#pragma once
#include "defines.h"

typedef enum mouse_buttons {
    MB_LEFT,
    MB_RIGHT,
    MB_MIDDLE,
    MB_MAX_BUTTONS
} mouse_buttons;

#define DEFINE_KEY(name,code) KEY_##name = code

/*
* Enumeration of key mappings. The key mappings are taken
* from the MSDN. Will be translated to other platforms
*/
typedef enum keys {
    DEFINE_KEY(BACK,0x08),
    DEFINE_KEY(TAB, 0x09),
    DEFINE_KEY(CLEAR, 0x0C),
    DEFINE_KEY(ENTER, 0x0D),
    DEFINE_KEY(SHIFT, 0x10),
    DEFINE_KEY(CONTROL, 0x11),
    DEFINE_KEY(ALT, 0x12),
    DEFINE_KEY(PAUSE, 0x13),
    DEFINE_KEY(CAPS, 0x14),
    DEFINE_KEY(ESCAPE, 0x1B),
    DEFINE_KEY(SPACE, 0x20),
    DEFINE_KEY(PAGE_UP, 0x21),
    DEFINE_KEY(PAGE_DOWN, 0x22),
    DEFINE_KEY(END, 0x23),
    DEFINE_KEY(HOME, 0x24),
    DEFINE_KEY(LEFT_ARR, 0x25),
    DEFINE_KEY(UP_ARR, 0x26),
    DEFINE_KEY(RIGHT_ARR, 0x27),
    DEFINE_KEY(DOWN_ARR, 0x28),
    DEFINE_KEY(SELECT, 0x29),
    DEFINE_KEY(PRINT, 0x2A),
    DEFINE_KEY(INSERT, 0x2D),
    DEFINE_KEY(DELETE, 0x2E),

    DEFINE_KEY(ZERO, 0x30),
    DEFINE_KEY(ONE, 0x31),
    DEFINE_KEY(TWO, 0x32),
    DEFINE_KEY(THREE, 0x33),
    DEFINE_KEY(FOUR, 0x34),
    DEFINE_KEY(FIVE, 0x35),
    DEFINE_KEY(SIZ, 0x36),
    DEFINE_KEY(SEVEN, 0x37),
    DEFINE_KEY(EIGHT, 0x38),
    DEFINE_KEY(NINE, 0x39),

    DEFINE_KEY(A, 0x41),
    DEFINE_KEY(B, 0x42),
    DEFINE_KEY(C, 0x43),
    DEFINE_KEY(D, 0x44),
    DEFINE_KEY(E, 0x45),
    DEFINE_KEY(F, 0x46),
    DEFINE_KEY(G, 0x47),
    DEFINE_KEY(H, 0x48),
    DEFINE_KEY(I, 0x49),
    DEFINE_KEY(J, 0x4A),
    DEFINE_KEY(K, 0x4B),
    DEFINE_KEY(L, 0x4C),
    DEFINE_KEY(M, 0x4D),
    DEFINE_KEY(N, 0x4E),
    DEFINE_KEY(O, 0x4F),
    DEFINE_KEY(P, 0x50),
    DEFINE_KEY(Q, 0x51),
    DEFINE_KEY(R, 0x52),
    DEFINE_KEY(S, 0x53),
    DEFINE_KEY(T, 0x54),
    DEFINE_KEY(U, 0x55),
    DEFINE_KEY(V, 0x56),
    DEFINE_KEY(W, 0x57),
    DEFINE_KEY(X, 0x58),
    DEFINE_KEY(Y, 0x59),
    DEFINE_KEY(Z, 0x5A),

    DEFINE_KEY(LWIN,0x5B),
    DEFINE_KEY(RWIN,0x5C),
    DEFINE_KEY(APPS,0x5D),
    DEFINE_KEY(SLEEP,0x5F),

    DEFINE_KEY(NUMPAD_ZERO,0x60),
    DEFINE_KEY(NUMPAD_ONE, 0x61),
    DEFINE_KEY(NUMPAD_TWO, 0x62),
    DEFINE_KEY(NUMPAD_THREE, 0x63),
    DEFINE_KEY(NUMPAD_FOUR, 0x64),
    DEFINE_KEY(NUMPAD_FIVE, 0x65),
    DEFINE_KEY(NUMPAD_SIX, 0x66),
    DEFINE_KEY(NUMPAD_SEVEN, 0x67),
    DEFINE_KEY(NUMPAD_EIGTH, 0x68),
    DEFINE_KEY(NUMPAD_NINE, 0x69),

    DEFINE_KEY(MULTIPLY, 0x6A),
    DEFINE_KEY(ADD, 0x6B),
    DEFINE_KEY(SEPARATOR, 0x6C),
    DEFINE_KEY(SUBTRACT, 0x6D),
    DEFINE_KEY(DECIMAL, 0x6E),
    DEFINE_KEY(DIVIDE, 0x6F),

    DEFINE_KEY(F1, 0x70),
    DEFINE_KEY(F2, 0x71),
    DEFINE_KEY(F3, 0x72),
    DEFINE_KEY(F4, 0x73),
    DEFINE_KEY(F5, 0x74),
    DEFINE_KEY(F6, 0x75),
    DEFINE_KEY(F7, 0x76),
    DEFINE_KEY(F8, 0x77),
    DEFINE_KEY(F9, 0x78),
    DEFINE_KEY(F10, 0x79),
    DEFINE_KEY(F11, 0x7A),
    DEFINE_KEY(F12, 0x7B),
    DEFINE_KEY(F13, 0x7C),
    DEFINE_KEY(F14, 0x7D),
    DEFINE_KEY(F15, 0x7E),
    DEFINE_KEY(F16, 0x7F),
    DEFINE_KEY(F17, 0x80),
    DEFINE_KEY(F18, 0x81),
    DEFINE_KEY(F19, 0x82),
    DEFINE_KEY(F20, 0x83),
    DEFINE_KEY(F21, 0x84),
    DEFINE_KEY(F22, 0x85),
    DEFINE_KEY(F23, 0x86),
    DEFINE_KEY(F24, 0x87),

    DEFINE_KEY(NUMLOCK, 0x90),
    DEFINE_KEY(SCROLL, 0x91),

    DEFINE_KEY(LSHIFT, 0xA0),
    DEFINE_KEY(RSHIFT, 0xA1),

    DEFINE_KEY(LCONTROL, 0xA2),
    DEFINE_KEY(RCONTROL, 0xA3),

    DEFINE_KEY(VOL_MUTE, 0xAD),
    DEFINE_KEY(VOL_DOWN, 0xAE),
    DEFINE_KEY(VOL_UP, 0xAF),

} keys;

/**
* Resonsible for initialization of the input sub-system.
*/
b8 input_initialize();

/**
* Responsible for shutting down the input system and cleaning up
* and pending input.
*/
void input_shutdown();

/**
* Responsible for updating the input state.
* 
* @param delta_time - The time since last frame
*/
void input_update(f64 delta_time);

// Keyboard input

/**
* Performing a check to see if a key is currently down or not.
* 
* @param key - The key to check
* @return b8 - TRUE if down, FALSE otherwise
*/
VAPI b8 input_is_key_down(keys key);

/**
* Performing a check to see if a key is currently up or not.
* 
* @param key - The key to check
* @return - TRUE if up, FALSE otherwise
*/
VAPI b8 input_is_key_up(keys key);

/**
* Performing a check to see if a key was down in the previous frame.
* 
* @param key - The key to check
* @return b8 - TRUE if it was down, FALSE otherwise
*/
VAPI b8 input_was_key_down(keys key);

/**
* Performing a check to see if a key was up in the previous frame
* 
* @param key - The key to check
* @return b8 - TRUE if it was up, FALSE otherwise
*/
VAPI b8 input_was_key_up(keys key);

/**
* Responsible for processing and updating key state.
* 
* @param key - The key to update
* @param pressed - TRUE if pressed, FALSE otherwise
*/
void input_process_key(keys key, b8 pressed);

// Mouse button input

/**
* Performing a check to see if a mouse button is down currently.
* 
* @param button - The mouse button to check
* @return b8 - TRUE if down, FALSE otherwise
*/
VAPI b8 input_is_button_down(mouse_buttons button);

/**
* Performing a check to see if a mouse button is up currently.
* 
* @param button - The mouse button to check
* @return b8 - TRUE if up, FALSE otherwise
*/
VAPI b8 input_is_button_up(mouse_buttons button);

/**
* Performing a check to see if a mouse button was down.
*
* @param button - The mouse button to check
* @return b8 - TRUE if down, FALSE otherwise
*/
VAPI b8 input_was_button_down(mouse_buttons button);

/**
* Performing a check to see if a mouse button was up.
*
* @param button - The mouse button to check
* @return b8 - TRUE if up, FALSE otherwise
*/
VAPI b8 input_was_button_up(mouse_buttons button);

/**
* Gets the current mouse position coordinates
*
* @param x - Pointer to store the result of the x coordinate
* @param y - Pointer to store the result of the y coordinate
*/
VAPI void input_get_mouse_position(i32* x, i32* y);

/**
* Gets the previous frame mouse position coordinates
*
* @param x - Pointer to store the result of the x coordinate
* @param y - Pointer to store the result of the y coordinate
*/
VAPI void input_get_previous_mouse_position(i32* x, i32* y);

/**
* Updates the state of the mouse buttons that are pressed.
*
* @param button - The button state to update
* @param pressed - TRUE if pressed, FALSE otherwise
*/
void input_process_button(mouse_buttons button, b8 pressed);

/**
* Updates the state of the mouse position.
*
* @param x - The new x coordinate of the mouse
* @param y - The new y coordinate of the mouse
*/
void input_process_mouse_move(i16 x, i16 y);

/**
* Updates the state of the mouse wheel delta.
*
* @param z_delta - The delta to indicate of the mouse wheel scrolled up or down
*/
void input_process_mouse_wheel(i8 z_delta);
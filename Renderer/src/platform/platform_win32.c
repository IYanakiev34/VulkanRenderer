#include "platform.h"

// Conditional compilation
#if R_PLATFORM_WINDOWS
#include <core/logger.h>
#include <stdlib.h>

#include <windows.h>
#include <windowsx.h> // param input extraction

typedef struct internal_state {
    HINSTANCE h_instance;
    HWND hwnd;
}internal_state;

// Clock
static f64 clock_frequency;
static LARGE_INTEGER start_time;

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);

b8 platform_startup(
    platform_state* plat_state,
    const char* application_name,
    i32 x,
    i32 y,
    i32 width,
    i32 height) {

    plat_state->internal_state = malloc(sizeof(internal_state));
    internal_state* state = (internal_state*)plat_state->internal_state;

    state->h_instance = GetModuleHandleA(0); // Get handle to current process

    HICON icon = LoadIcon(state->h_instance, IDI_APPLICATION);
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(WNDCLASSA));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = win32_process_message;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = "tiny_window_class";

    if (!RegisterClassA(&wc)) {
        MessageBoxA(0, "Window registration failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    // Window borders
    u32 client_x = x;
    u32 client_y = y;
    u32 client_width = width;
    u32 client_height = height;

    u32 window_x = client_x;
    u32 window_y = client_y;
    u32 window_width = client_width;
    u32 window_height = client_height;

    // Window styles
    u32 window_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    u32 window_ex_style = WS_EX_APPWINDOW;

    window_style |= WS_MAXIMIZEBOX;
    window_style |= WS_MINIMIZEBOX;
    window_style |= WS_THICKFRAME;

    RECT border_rect = { 0,0,0,0 };
    AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

    // In this case they will benegative to audjust for client side
    window_x += border_rect.left;
    window_y += border_rect.top;

    window_width += border_rect.right - border_rect.left;
    window_height += border_rect.bottom - border_rect.top;

    // Create the window
    HWND handle = CreateWindowExA(
        window_ex_style, "tiny_window_class", application_name,
        window_style, window_x, window_y, window_width, window_height,
        0, 0, state->h_instance, 0);

    if (handle == 0)
    {
        MessageBoxA(NULL, "Window Creation Failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        VFATAL("Window creation failure");
        return FALSE;
    }

    state->hwnd = handle;

    // Show the window
    b32 should_activate = 1; // TODO: Should be parameter toindicate if we want to take input
    i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;

    // if intially minimized:  SW_MINIMIZE : SW_SHOWMINNOACTIVE;
    // if maximized: SW_MAXIMIZE : SW_SHOWMAXIMIZED

    ShowWindow(state->hwnd, show_window_command_flags);

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency = 1.f / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&start_time);

    return TRUE;
}

void platform_shutdown(platform_state* plat_state) {
    internal_state* state = (internal_state*)plat_state->internal_state;

    if (state->hwnd)
    {
        DestroyWindow(state->hwnd);
        state->hwnd = 0;
    }
}

b8 platform_pump_message(platform_state* plat_state){
    MSG message;
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return TRUE;
}

void* platform_allocate(u64 size, b8 aligned) {
    return malloc(size); // Temporary solution
}

void platform_free(void* block, b8 aligned) {
    free(block);
}

void* platform_zero_memory(void* block, u64 size) {
    memset(block, 0, size);
}

void* platform_copy_memory(void* dest, void* src, u64 size) {
    memcpy(dest, src, size);
}

void* platform_set_memory(void* block, i32 value, u64 size) {
    memset(block, value, size);
}

void platform_console_write(const char* msg, u8 color) {
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    // TRACE, DEBUG, INFO, WARN, ERROR, FATAL Colors using the windows api
    static u8 levels[6] = { 8, 1, 2, 6, 4, 64 };
    SetConsoleTextAttribute(console_handle, levels[color]);

    OutputDebugStringA(msg); // Output to debug console

    // Output to the standard console
    u64 length = strlen(msg);
    LPDWORD number_written = 0;
    WriteConsoleA(console_handle, msg, (DWORD)length, number_written, 0);
}

void platform_console_write_error(const char* msg, u8 color) {
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
    // TRACE, DEBUG, INFO, WARN, ERROR, FATAL Colors using the windows api
    static u8 levels[6] = { 8, 1, 2, 6, 4, 64 };
    SetConsoleTextAttribute(console_handle, levels[color]);

    OutputDebugStringA(msg); // Output to debug console

    // Output to the standard console
    u64 length = strlen(msg);
    LPDWORD number_written = 0;
    WriteConsoleA(console_handle, msg, (DWORD)length, number_written, 0);
}

f64 platform_get_absolute_time() {
    LARGE_INTEGER now_time;
    QueryPerformanceCounter(&now_time);
    return (f64)now_time.QuadPart * clock_frequency;
}

void platform_sleep(u64 ms){
    Sleep(ms);
}

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param) {
    switch (msg) {
    case WM_CLOSE:
        // TODO: Fire an event for the application to quit
        return 0; // Let the OS know we have handled it
    case WM_ERASEBKGND:
        // Notify the OS that erasing will be handled by the application not the OS
        return 1;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE: {
        // Get the update size
        RECT r;
        GetClientRect(hwnd, &r);
        u32 width = r.right - r.left;
        u32 height = r.bottom - r.top;

        // TODO: fire event for window resize
        }break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP: {
        b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
        b8 relaesed = (msg == WM_KEYUP || msg == WM_SYSKEYUP);
        // TODO: input processing here
    } break;
    case WM_MOUSEMOVE: {
        i32 x_position = GET_X_LPARAM(l_param);
        i32 y_position = GET_Y_LPARAM(l_param);
        // TODO: input processing
    } break;
    case WM_MOUSEWHEEL: {
        i32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
        if (z_delta != 0) {
            z_delta = (z_delta < 0) ? -1 : 1; // Flatten value
            // TODO: process input
        }
    } break;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP: {
        b8 is_pressed = (msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_RBUTTONDOWN);
        b8 is_released = (msg == WM_LBUTTONUP || msg == WM_MBUTTONUP || msg == WM_RBUTTONUP);
        // TODO: process input processing on this
    } break;

    }

    return DefWindowProcA(hwnd, msg, w_param, l_param);
}

#endif
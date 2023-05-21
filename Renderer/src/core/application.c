#include "application.h"

// Systems
#include "logger.h"
#include "vmemory.h"
#include "event.h"
#include "input.h"

// Resources
#include "game_types.h"
#include "clock.h"
#include "vstring.h"

#include "platform/platform.h"

// Renderer
#include "renderer/renderer_frontend.h"


typedef struct application_state {
    game* game_inst;
    b8 is_running;
    b8 is_suspended;
    platform_state platform;
    i32 width;
    i32 height;
    clock clock;
    f64 last_time;
} application_state;

static b8 initialized = FALSE;
static application_state app_state;

// Event handlers
b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context data);
b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context data);

b8 application_create(game* game_inst) {
    if (initialized) {
        VERROR("application_create called more than once.");
        return FALSE;
    }

    app_state.game_inst = game_inst;

    // Intialize subsystems
    {
        if (!intialize_logging()) {
            VFATAL("Logging system failed initialization. Application cannot continue");
            return FALSE;
        }

        if (!input_initialize()) {
            VFATAL("Input system failed initialization. Application cannot continue");
            return FALSE;
        }

        if (!event_initialize()) {
            VFATAL("Event system failed initialization. Application cannot continue");
            return FALSE;
        }
    }

    // Set app state
    {
        app_state.is_running = TRUE;
        app_state.is_suspended = FALSE;
    }

    // Register for events
    {
        event_register(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
        event_register(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
        event_register(EVENT_CODE_KEY_RELEASED, 0, application_on_key);
    }
    

    // Failed to initialize platform
    if (!platform_startup(
        &app_state.platform,
        game_inst->app_config.name,
        game_inst->app_config.start_x,
        game_inst->app_config.start_y,
        game_inst->app_config.start_width,
        game_inst->app_config.start_height))
    {
        VFATAL("Could not initialize the platform")
        return FALSE;
    }

    // Initialize renderer
    {
        if (!renderer_initialize("Tiny Vulkan Renderer", &app_state.platform)) {
            VFATAL("Could not initialize renderer. Aborting application...");
            return FALSE;
        }
    }

    // Initialize the game
    {
        if (!app_state.game_inst->initialize(app_state.game_inst))
        {
            VFATAL("Could not intialize the game!");
            return FALSE;
        }
    }

    // Hook up on resize callback for the game
    // TODO: implement window resizing
    app_state.game_inst->on_resize(app_state.game_inst, app_state.width, app_state.height);

    initialized = TRUE;
    return TRUE;
}

b8 application_run() {
    clock_start(&app_state.clock);
    clock_update(&app_state.clock);
    app_state.last_time = app_state.clock.elapsed_time;
    f64 running_time = 0;
    u8 frame_count = 0;
    f64 target_frame_time = 1.f / 60.f;

    char* memory_usage = get_memory_usage_str();
    VINFO(memory_usage); // HACK: leaks memory

    while (app_state.is_running)
    {
        if (!platform_pump_message(&app_state.platform))
            app_state.is_running = FALSE;
        
        // If the game is not paused
        if (!app_state.is_suspended)
        {
            clock_update(&app_state.clock);
            f64 current_time = app_state.clock.elapsed_time;
            f64 delta_time = (current_time - app_state.last_time);
            f64 frame_start_time = platform_get_absolute_time();

            // Update game
            if (!app_state.game_inst->update(app_state.game_inst, delta_time))
            {
                VFATAL("Could not update the game!");
                app_state.is_running = FALSE;
                break;
            }

            // Render the game
            if (!app_state.game_inst->render(app_state.game_inst, delta_time))
            {
                VFATAL("Could not render the game!");
                app_state.is_running = FALSE;
                break;
            }

            // TODO: this should not be like this
            render_packet packet;
            packet.delta_time = delta_time;
            if (!renderer_draw_frame(&packet)) {
                VERROR("renderer_draw_frame returned false. Could not draw frame: %u", frame_count);
            }

            // Calculation to find how long the frame needed
            {
                f64 frame_end_time = platform_get_absolute_time();
                f64 frame_elapsed_time = frame_end_time - frame_start_time;
                running_time += frame_elapsed_time;
                f64 remaining_seconds = target_frame_time - frame_elapsed_time;

                // Give a bit back to system if we reached target frame time
                if (remaining_seconds > 0.0000) {
                    u64 remaining_ms = ((u64)remaining_seconds * 1000);

                    b8 limit_frames = FALSE;// TODO: configurable
                    if (remaining_ms > 0 && limit_frames) {
                        platform_sleep(remaining_ms);
                    }

                    ++frame_count;
                }
            }
            
            input_update(delta_time);

            // Update last time
            app_state.last_time = current_time;
        }
    }

    app_state.is_running = FALSE;
    
    // Deregister from events
    {
        event_unregister(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
        event_unregister(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
        event_unregister(EVENT_CODE_KEY_RELEASED, 0, application_on_key);
    }

    // Shutdown systems
    {
        VINFO("Shutting down event system...");
        event_shutdown();
        VINFO("Shutting down input system...");
        input_shutdown();
        VINFO("Shutting down renderer system...");
        renderer_shutdown();
        VINFO("Shutting down the platform...");
        platform_shutdown(&app_state.platform);
        VINFO("Shutting down logging system...");
        shutdown_logging(); // Logging after platform since we might want to log final stuff to platform
    }

    memory_usage = get_memory_usage_str();
    VINFO(memory_usage); // HACK: leaks memory

    return TRUE;
}

b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context data) {
    if (code == EVENT_CODE_APPLICATION_QUIT) {
        VINFO("EVENT_CODE_APPLICATION_QUIT received, shutting down!");
        app_state.is_running = FALSE;
        return TRUE;
    }

    return FALSE;
}

b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context data) {
    u16 key_code = data.data.u16[0];
    if (code == EVENT_CODE_KEY_PRESSED) {
        if (key_code == KEY_ESCAPE) {
            // Fire event to self. Maybe other listeners also
            event_context event = {0};
            event_fire(EVENT_CODE_APPLICATION_QUIT, 0, event);
            return TRUE;
        }
        else if (key_code == KEY_A) {
            VDEBUG("Key 'A' was pressed");
        }
        else {
            VDEBUG("'%c' key was pressed", key_code);
        }
    }
    else if (code == EVENT_CODE_KEY_RELEASED) {
        if (key_code == KEY_B) {
            VDEBUG("Key 'B' was released");
        }
        else {
            VDEBUG("'%c' key was released", key_code);
        }
    }

    return FALSE;
}
#include "logger.h"
#include "game_types.h"

#include "application.h"
#include "platform/platform.h"
#include "core/vmemory.h"
#include "core/event.h"


typedef struct application_state {
    game* game_inst;
    b8 is_running;
    b8 is_suspended;
    platform_state platform;
    i32 width;
    i32 height;
    f64 last_time;
} application_state;

static b8 initialized = FALSE;
static application_state app_state;

b8 application_create(game* game_inst) {
    if (initialized) {
        VERROR("application_create called more than once.");
        return FALSE;
    }

    app_state.game_inst = game_inst;

    // Intialize subsystems
    intialize_logging();

    // TEST logging
    // TODO: remove it later
    {
        VTRACE("A test message: %f", 3.14);
        VDEBUG("A test message: %f", 3.14);
        VINFO("A test message: %f", 3.14);
        VWARN("A test message: %f", 3.14);
        VERROR("A test message: %f", 3.14);
        VFATAL("A test message: %f", 3.14);
    }

    app_state.is_running = TRUE;
    app_state.is_suspended = FALSE;

    if (!event_initialize()) {
        VFATAL("Event system failed initialization. Application cannot continue");
        return FALSE;
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

    // Initialize the game
    if (!app_state.game_inst->initialize(app_state.game_inst))
    {
        VFATAL("Could not intialize the game!");
        return FALSE;
    }

    // Hook up on resize callback for the game
    // TODO: implement window resizing
    app_state.game_inst->on_resize(app_state.game_inst, app_state.width, app_state.height);

    initialized = TRUE;

    return TRUE;
}

b8 application_run() {
    char* memory_usage = get_memory_usage_str();
    VINFO(memory_usage);
    while (app_state.is_running)
    {
        if (!platform_pump_message(&app_state.platform))
            app_state.is_running = FALSE;
        
        // If the game is not paused
        if (!app_state.is_suspended)
        {
            // Update game
            if (!app_state.game_inst->update(app_state.game_inst, (f32)0))
            {
                VFATAL("Could not update the game!");
                app_state.is_running = FALSE;
                break;
            }

            // Render the game
            if (!app_state.game_inst->render(app_state.game_inst, (f32)0))
            {
                VFATAL("Could not render the game!");
                app_state.is_running = FALSE;
                break;
            }
        }
    }

    app_state.is_running = FALSE;

    event_shutdown();
    platform_shutdown(&app_state.platform);

    return TRUE;
}
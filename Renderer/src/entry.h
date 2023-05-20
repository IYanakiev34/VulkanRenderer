#pragma once

#include "core/application.h"
#include "core/logger.h"
#include "game_types.h"
#include "core/vmemory.h"

extern b8 create_game(game* out_game);

/**
* The main entry point of the application
*/
int main(void)
{
    VINFO("Initializing memory system...");
    initialize_memory();

    game game_inst;// Create game
    if (!create_game(&game_inst))
    {
        VFATAL("Could not initialize game!");
        return -1;
    }

    // Ensure function pointers are set
    if (!game_inst.render || !game_inst.update || !game_inst.on_resize)
    {
        VFATAL("Missing function pointers on the game object");
        return -2;
    }

    if (!application_create(&game_inst))
    {
        VFATAL("Could not create the application");
        return 1;
    }

    if (!application_run())
    {
        VINFO("The application did not shutdown gracefully!");
        return 2;
    }

    shutdown_memory();
    return 0;
}
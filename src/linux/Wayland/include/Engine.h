#pragma once

#include "Window.h"
#include "Input.h"
#include "Game.h"
#include "Export.h"

namespace Luna
{
    class DLL Engine
    {
    private:
        int32 Loop();

        static bool quit;
        static void Quit(void *data, struct xdg_toplevel *toplevel);
        static void Display(void *data, wl_callback *callback, uint32 time);

    public:
        static Window * window;
        static Input * input;
        static Game * game;
        
        explicit Engine() noexcept;
        ~Engine() noexcept;

        int32 Start(Game * const game);
    };
}
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

    public:
        static Window * window;
        static Input * input;
        static Game * game;

        explicit Engine() noexcept;
        ~Engine() noexcept;

        int32 Start(Game * const game);

        static void EngineProc(XEvent * event);
    };
}
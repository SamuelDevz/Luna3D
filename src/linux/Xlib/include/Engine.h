#pragma once

#include "Window.h"
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
        static Game * game;

        explicit Engine() noexcept;
        ~Engine() noexcept;

        int32 Start(Game * const game);
    };
}
#pragma once

#include "Window.h"
#include "Game.h"

namespace Luna
{
    class Engine
    {
    private:
        static Game * game;
        int32 Loop();

    public:
        static Window * window;

        explicit Engine() noexcept;
        ~Engine() noexcept;

        int32 Start(Game * const game);
        
        static LRESULT CALLBACK EngineProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };
}
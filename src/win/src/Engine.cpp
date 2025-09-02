#include "Engine.h"
#include "WinInclude.h"
#include <windows.h>
#include <format>
using std::format;

namespace Luna 
{
    Window*   Engine::window = nullptr;
    Input*    Engine::input = nullptr;
    Game*     Engine::game = nullptr;

    Engine::Engine() noexcept
    {
        window = new Window();
    }

    Engine::~Engine() noexcept
    {
        delete game;
        delete input;
        delete window;
    }

    int32 Engine::Start(Game * const game)
    {
        this->game = game;

        window->Create();

        input = new Input();

        SetWindowLongPtr(window->Id(), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(EngineProc));

        return Loop();
    }

    int32 Engine::Loop()
    {
        MSG msg{};

        game->Init();

        do
        {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                game->Update();
                game->Draw();
            }
        } while (msg.message != WM_QUIT);

        game->Finalize();    

        return int32(msg.wParam);
    }

    LRESULT CALLBACK Engine::EngineProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (msg == WM_PAINT)
            game->Display();

        return CallWindowProc(Input::InputProc, hWnd, msg, wParam, lParam);
    }
}
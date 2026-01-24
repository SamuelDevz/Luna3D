#include "Engine.h"

namespace Luna 
{
    Window*   Engine::window = nullptr;
    Game*     Engine::game = nullptr;
    bool      Engine::quit = false;

    Engine::Engine() noexcept
    {
        window = new Window();
    }

    Engine::~Engine() noexcept
    {
        delete game;
        delete window;
    }

    int32 Engine::Start(Game * const game)
    {
        this->game = game;

        window->Create();

        return Loop();
    }

    void Engine::Quit(void *data, xdg_toplevel *toplevel)
    {
        quit = true;
    }

    int32 Engine::Loop()
    {
        game->Init();
        window->OnClose(Quit);

        do
        {
            game->Update();
            game->Draw();
        } while (wl_display_dispatch(window->Display()) && !quit);

        game->Finalize();

        return 0;
    }
}
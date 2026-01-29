#include "Engine.h"

namespace Luna 
{
    Window*   Engine::window = nullptr;
    Input*    Engine::input = nullptr;
    Game*     Engine::game = nullptr;
    bool      Engine::quit = false;

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
        window->OnDisplay(Display);
        input->Initialize(window->Display());

        do
        {
            while (wl_display_prepare_read(window->Display()) != 0)
                wl_display_dispatch_pending(window->Display());
            wl_display_flush(window->Display());
            wl_display_read_events(window->Display());
            wl_display_dispatch_pending(window->Display());

            game->Update();
            game->Draw();
        } while (wl_display_dispatch(window->Display()) && !quit);

        game->Finalize();

        return 0;
    }

    void Engine::Display(void *data, wl_callback *callback, uint32 time)
    {
        wl_callback_destroy(callback);
        game->Display();
        
        static const wl_callback_listener frameListener = {
            .done = Display
        };
        
        wl_callback *nextCallback = wl_surface_frame(window->Surface());
        wl_callback_add_listener(nextCallback, &frameListener, nullptr);
        wl_surface_commit(window->Surface());
    }
}
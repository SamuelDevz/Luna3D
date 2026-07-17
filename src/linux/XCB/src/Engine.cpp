#include "Engine.h"

namespace Luna 
{
    Window*   Engine::window = nullptr;
    Game*     Engine::game = nullptr;

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

    bool Quit(xcb_generic_event_t *event, xcb_atom_t wmDeleteWindow)
    {
        auto message = reinterpret_cast<xcb_client_message_event_t*>(event);
        if(message->data.data32[0] == wmDeleteWindow)
            return true;
        return false;
    }

    int32 Engine::Loop()
    {
        xcb_generic_event_t *event;
        game->Init();

        do
        {
            event = xcb_wait_for_event(window->Connection());
            game->Update();
            game->Draw();
        } while (!Quit(event, window->WMDeleteWindow()));
        
        delete event;

        game->Finalize();

        return 0;
    }
}
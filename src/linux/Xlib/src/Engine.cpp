#include "Engine.h"
#include <X11/Xlib.h>

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

    bool Quit(const XEvent * event, const Atom wmDeleteWindow)
    {
        if(event->type == ClientMessage)
            if(event->xclient.data.l[0] == wmDeleteWindow)
                return true;
        return false;
    }

    int32 Engine::Loop()
    {
        XEvent event{};
        game->Init();

        do
        {
            XNextEvent(window->XDisplay(), &event);
            game->Update();
            game->Draw();
        } while (!XPending(window->XDisplay()) && !Quit(&event, window->WMDeleteWindow()));

        game->Finalize();

        return 0;
    }
}
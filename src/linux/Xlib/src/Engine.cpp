#include "Engine.h"

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

        input->Initialize(window->XDisplay(), window->Id(), &event);

        do
        {
            XNextEvent(window->XDisplay(), &event);
            EngineProc(&event);
            game->Update();
            game->Draw();
        } while (!Quit(&event, window->WMDeleteWindow()));

        game->Finalize();

        return 0;
    }

    void Engine::EngineProc(XEvent * event)
    {
        if (event->type == Expose)
            game->Display();

        Input::InputProc(event);
    }
}
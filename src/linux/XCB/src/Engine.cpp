#include "Engine.h"
#include "KeyCodes.h"
#include <format>
using std::format;

namespace Luna 
{
    Window*   Engine::window = nullptr;
    Input*    Engine::input = nullptr;
    Game*     Engine::game = nullptr;
    double    Engine::frameTime = {};
    bool      Engine::paused = false;
    Timer     Engine::timer;

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

    double Engine::FrameTime() noexcept
    {
    #ifdef _DEBUG
        static double totalTime{};
        static uint32 frameCount{};
    #endif

        double newFrameTime = timer.Reset();
        if (newFrameTime <= 1.0)
            frameTime = newFrameTime;

    #ifdef _DEBUG
        totalTime += frameTime;

        frameCount++;

        if (totalTime >= 1.0)
        {
            string title = format("{}    FPS: {}    Frame Time: {:.3f} (ms)",
                window->Title().c_str(), frameCount, frameTime * 1000).c_str();

            xcb_change_property(window->Connection(),
                XCB_PROP_MODE_REPLACE,
                window->Id(),
                XCB_ATOM_WM_NAME,
                XCB_ATOM_STRING,
                8,
                title.size(),
                title.c_str());

            xcb_flush(window->Connection());

            frameCount = 0;
            totalTime -= 1.0;
        }
    #endif

        return frameTime;
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

        input->Initialize(window->Connection(), window->Id(), event);

        do
        {
            while ((event = xcb_poll_for_event(window->Connection())) != nullptr)
            {
                if (Quit(event, window->WMDeleteWindow()))
                {
                    delete event;
                    game->Finalize();
                    return 0;
                }
                
                EngineProc(event);
                
                if (input->XKeyPress(VK_PAUSE))
                    (paused) ? Resume() : Pause();
                
                delete event;
            }
            
            if (!paused)
            {
                frameTime = FrameTime();
                game->Update();
                game->Draw();
            }
            else
            {
                game->OnPause();
            }
        } while (true);

        return 0;
    }

    void Engine::EngineProc(xcb_generic_event_t * event)
    {
        if (event->response_type == XCB_EXPOSE)
            game->Display();

        Input::InputProc(event);
    }
}
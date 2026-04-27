#include "Engine.h"
#include "KeyCodes.h"
#include <X11/Xatom.h>
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

        frameTime = timer.Reset();

    #ifdef _DEBUG
        totalTime += frameTime;

        frameCount++;

        if (totalTime >= 1.0)
        {
            string title = format("{}    FPS: {}    Frame Time: {:.3f} (ms)",
                window->Title().c_str(), frameCount, frameTime * 1000);

            static Atom _NET_WM_NAME = XInternAtom(window->XDisplay(), "_NET_WM_NAME", false);
            static Atom UTF8_STRING = XInternAtom(window->XDisplay(), "UTF8_STRING", false);

            XChangeProperty(
                window->XDisplay(),
                window->Id(),
                _NET_WM_NAME,
                UTF8_STRING,
                8,
                PropModeReplace,
                reinterpret_cast<const uint8*>(title.c_str()),
                static_cast<int32>(title.size())
            );

            XChangeProperty(
                window->XDisplay(),
                window->Id(),
                XA_WM_NAME,
                XA_STRING,
                8,
                PropModeReplace,
                reinterpret_cast<const uint8*>(title.c_str()),
                static_cast<int32>(title.size())
            );

            frameCount = 0;
            totalTime -= 1.0;
        }
    #endif

        return frameTime;
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
        timer.Start();
        XEvent event{};
        game->Init();
        input->Initialize(window->XDisplay(), window->Id(), &event);

        do
        {
            while (XPending(window->XDisplay()) > 0)
            {
                XNextEvent(window->XDisplay(), &event);
                
                if (input->XKeyPress(VK_PAUSE))
                    (paused) ? Resume() : Pause();
                
                EngineProc(&event);
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
        } while (!Quit(&event, window->WMDeleteWindow()));

        game->Finalize();

        return 0;
    }

    void Engine::EngineProc(const XEvent * const event)
    {
        if (event->type == Expose)
            game->Display();

        Input::InputProc(event);
    }
}
#include "Engine.h"
#include "KeyCodes.h"
#include <cstdio>
#include <cstdarg>
#include <format>
using std::format;

namespace Luna 
{
    Window*   Engine::window = nullptr;
    Input*    Engine::input = nullptr;
    Game*     Engine::game = nullptr;
    bool      Engine::quit = false;
    bool      Engine::paused = false;
    double    Engine::frameTime = {};
    Timer     Engine::timer;

    static void WaylandLogHandler(const char* fmt, va_list args) 
    {
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
    }

    Engine::Engine() noexcept
    {
        wl_log_set_handler_client(WaylandLogHandler);
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
                window->Title().c_str(), frameCount, frameTime * 1000).c_str();

            xdg_toplevel_set_title(window->XDGTopLevel(), title.c_str());

            frameCount = 0;
            totalTime -= 1.0;
        }
    #endif

        return frameTime;
    }

    int32 Engine::Loop()
    {
        timer.Start();
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

            if (input->KeyPress(VK_PAUSE))
                (paused) ? Resume() : Pause();

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
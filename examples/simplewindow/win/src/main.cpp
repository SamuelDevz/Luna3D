#include "WinGame.h"
#include "Resources.h"

auto APIENTRY WinMain(_In_ HINSTANCE hInstance, 
    _In_opt_ HINSTANCE hPrevInstance, 
    _In_ LPSTR lpCmdLine, 
    _In_ int nCmdShow) -> int
{
    using namespace Luna;

#ifdef _DEBUG

    try
    {
        Engine * engine = new Engine();
        engine->window->Mode(WINDOWED);
        engine->window->Size(800, 600);
        engine->window->Color(0, 122, 204);
        engine->window->Title("Window Game");
        engine->window->Icon(IDI_ICON);
        engine->window->Cursor(IDC_CURSOR);
        engine->window->LostFocus(Engine::Pause);
        engine->window->InFocus(Engine::Resume);

        int exit = engine->Start(new WinGame());

        delete engine;

        return exit;
    }
    catch (const Error & e)
    {
        MessageBox(nullptr, e.ToString().c_str(), "Window Game", MB_OK);
        return 0;
    }

#else

    Engine * engine = new Engine();
    engine->window->Mode(WINDOWED);
    engine->window->Size(800, 600);
    engine->window->Color(0, 122, 204);
    engine->window->Title("Window Game");
    engine->window->Icon(IDI_ICON);
    engine->window->Cursor(IDC_CURSOR);
    engine->window->LostFocus(Engine::Pause);
    engine->window->InFocus(Engine::Resume);

    int exit = engine->Start(new WinGame());

    delete engine;

    return exit;

#endif
}
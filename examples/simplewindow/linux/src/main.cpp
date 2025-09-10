#include "WinGame.h"
#include "Resource.h"

int main()
{
    using namespace Luna;

#ifdef _DEBUG

    try
    {
        Engine * engine = new Engine();
        engine->window->Mode(WINDOWED);
        engine->window->Size(800, 600);
        engine->window->Color("#007acc");
        engine->window->Title("Window Game");
        engine->window->Icon(GetIconFile());
        engine->window->Cursor(GetCursorFile());

        int exit = engine->Start(new WinGame());

        delete engine;

        return exit;
    }
    catch (Error & e)
    {
        MessageBox("Window Game", e.ToString().c_str());
        return 0;
    }

#else

    Engine * engine = new Engine();
    engine->window->Mode(WINDOWED);
    engine->window->Size(800, 600);
    engine->window->Color("#007acc");
    engine->window->Title("Window Game");
    engine->window->Icon(GetIconFile());
    engine->window->Cursor(GetCursorFile());

    int exit = engine->Start(new WinGame());

    delete engine;

    return exit;

#endif
}
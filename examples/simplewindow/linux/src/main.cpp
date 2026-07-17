#include "WinGame.h"
#include "Resource.h"

int main()
{
    using namespace Luna;

    Engine * engine = new Engine();
    engine->window->Mode(WINDOWED);
    engine->window->Size(800, 600);
    engine->window->Color("#007ACC");
    engine->window->Title("Window Game");
    engine->window->Icon(GetIconFile());
    engine->window->Cursor(GetCursorFile());

    int exit = engine->Start(new WinGame());

    delete engine;

    return exit;
}
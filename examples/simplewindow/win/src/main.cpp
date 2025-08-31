#include "WinGame.h"
#include "Resources.h"

auto APIENTRY WinMain(_In_ HINSTANCE hInstance, 
    _In_opt_ HINSTANCE hPrevInstance, 
    _In_ LPSTR lpCmdLine, 
    _In_ int nCmdShow) -> int
{
    using namespace Luna;

    Engine * engine = new Engine();
    engine->window->Mode(WINDOWED);
    engine->window->Size(800, 600);
    engine->window->Color(0, 122, 204);
    engine->window->Title("Window Game");
    engine->window->Icon(IDI_ICON);
    engine->window->Cursor(IDC_CURSOR);

    int exit = engine->Start(new WinGame());

    delete engine;

    return exit;
}
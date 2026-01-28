#include "Game.h"
#include "Engine.h"

namespace Luna
{
    Window*   & Game::window    = Engine::window;
    Input*    & Game::input     = Engine::input;
    
    Game::Game() noexcept
    {
    }

    Game::~Game()
    {
    }
}
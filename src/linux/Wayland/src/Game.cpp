#include "Game.h"
#include "Engine.h"

namespace Luna
{
    Window*   & Game::window    = Engine::window;
    Input*    & Game::input     = Engine::input;
    double    & Game::frameTime = Engine::frameTime;
    
    Game::Game() noexcept
    {
    }

    Game::~Game()
    {
    }
}
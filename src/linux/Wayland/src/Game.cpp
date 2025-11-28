#include "Game.h"
#include "Engine.h"

namespace Luna
{
    Window*   & Game::window    = Engine::window;
    
    Game::Game() noexcept
    {
    }

    Game::~Game()
    {
    }
}
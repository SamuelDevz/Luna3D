#pragma once

#include "Graphics.h"
#include "Window.h"
#include "Input.h"
#include "Export.h"

namespace Luna
{
    class DLL Game
    {
    protected:
        static Graphics *& graphics;
        static Window*   & window;
        static Input*    & input;
        static double    & frameTime;
        
    public:
        explicit Game() noexcept;
        virtual ~Game();

        virtual void Init() = 0;
        virtual void Update() = 0;
        virtual void Finalize() = 0;

        virtual void Draw() {}
        virtual void Display() {}
        virtual void OnPause() { Sleep(10); }
    };
}
#pragma once

#include "Types.h"
#include "Window.h"
#include "Export.h"

namespace Luna
{
    enum { MAX_KEYS = 256 };

    class DLL Input
    {
    private:
        static bool	keys[MAX_KEYS];
        static bool ctrl[MAX_KEYS];
        static string text;

        static int32 mouseX;
        static int32 mouseY;
        static int16 mouseWheel;

    public:
        explicit Input() noexcept;
        ~Input() noexcept;

        bool KeyDown(const int32 vkcode) const noexcept;
        bool KeyUp(const int32 vkcode) const noexcept;
        bool KeyPress(const int32 vkcode) noexcept;

        int32 MouseX() const noexcept;
        int32 MouseY() const noexcept;
        int16 MouseWheel() noexcept;

        void Read() noexcept;
        static const char* Text() noexcept;

        static LRESULT CALLBACK Reader(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK InputProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };

    inline bool Input::KeyDown(const int32 vkcode) const noexcept
    { return keys[vkcode]; }

    inline bool Input::KeyUp(const int32 vkcode) const noexcept
    { return !(keys[vkcode]); }

    inline int32 Input::MouseX() const noexcept
    { return mouseX; }

    inline int32 Input::MouseY() const noexcept
    { return mouseY; }

    inline const char* Input::Text() noexcept
    { return text.c_str(); }
}
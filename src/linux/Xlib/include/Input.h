#pragma once

#include "Types.h"
#include "Export.h"
#include "Window.h"

namespace Luna 
{
    enum { MAX_KEYS = 256 };

    class DLL Input
    {
    private:
        static Display * display;
        static XWindow window;
        static XEvent * event;

        static bool	keys[MAX_KEYS];
        static bool ctrl[MAX_KEYS];
        static string text;

        static int32 mouseX;
        static int32 mouseY;
        static int16 mouseWheel;

        static XIM xim;
        static XIC xic;

    public:
        ~Input() noexcept;

        void Initialize(Display * display, XWindow window, XEvent * event);

        bool KeyDown(const uint32 vkcode) const noexcept;
        bool KeyUp(const uint32 vkcode) const noexcept;
        bool XKeyPress(const uint32 vkcode) noexcept;

        int32 MouseX() const noexcept;
        int32 MouseY() const noexcept;
        int16 MouseWheel() noexcept;

        void Read() noexcept;
        static const char* Text() noexcept;

        static void Reader(XEvent * event);
        static void InputProc(XEvent * event);
    };

    inline bool Input::KeyDown(const uint32 vkcode) const noexcept
    { return keys[XKeysymToKeycode(display, vkcode)]; }

    inline bool Input::KeyUp(const uint32 vkcode) const noexcept
    { return !(keys[XKeysymToKeycode(display, vkcode)]); }

    inline int32 Input::MouseX() const noexcept
    { return mouseX; }

    inline int32 Input::MouseY() const noexcept
    { return mouseY; }

    inline const char* Input::Text() noexcept
    { return text.c_str(); }
}
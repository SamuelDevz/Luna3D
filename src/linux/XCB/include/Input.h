#pragma once

#include "Types.h"
#include "Export.h"
#include "Window.h"
#include <xcb/xcb_keysyms.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <xkbcommon/xkbcommon-compose.h>

namespace Luna
{
    enum { MAX_KEYS = 256 };

    class DLL Input
    {
    private:
        static xcb_key_symbols_t* keysyms;
        static xcb_generic_event_t* event;
        static xcb_connection_t* connection;
        static xcb_window_t window;

        static bool	keys[MAX_KEYS];
        static bool ctrl[MAX_KEYS];
        static string text;

        static int32 mouseX;
        static int32 mouseY;
        static int16 mouseWheel;

        static xkb_context * context;
        static xkb_keymap * keymap;
        static xkb_state * state;

        static xkb_compose_table * composeTable;
        static xkb_compose_state * composeState;

        xkb_keycode_t KeysymToKeycode(const xkb_keysym_t keysym) const noexcept;

    public:
        ~Input() noexcept;

        void Initialize(xcb_connection_t* connection, xcb_window_t window, xcb_generic_event_t* event);

        bool KeyDown(const uint32 vkcode) const noexcept;
        bool KeyUp(const uint32 vkcode) const noexcept;
        bool XKeyPress(const uint32 vkcode) noexcept;

        int32 MouseX() const noexcept;
        int32 MouseY() const noexcept;
        int16 MouseWheel() noexcept;

        void Read() noexcept;
        static const char* Text() noexcept;

        static void Reader(xcb_generic_event_t * event);
        static void InputProc(xcb_generic_event_t * event);
    };

    inline bool Input::KeyDown(const uint32 vkcode) const noexcept
    { return keys[KeysymToKeycode(vkcode)]; }

    inline bool Input::KeyUp(const uint32 vkcode) const noexcept
    { return !(keys[KeysymToKeycode(vkcode)]); }

    inline int32 Input::MouseX() const noexcept
    { return mouseX; }

    inline int32 Input::MouseY() const noexcept
    { return mouseY; }

    inline const char* Input::Text() noexcept
    { return text.c_str(); }
}
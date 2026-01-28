#pragma once

#include "Types.h"
#include "Export.h"
#include "Window.h"
#include <xkbcommon/xkbcommon.h>

namespace Luna
{
    enum { MAX_KEYS = 256 };

    class DLL Input
    {
    private:
        static wl_seat* seat;
        static wl_keyboard* keyboard;

        static bool keys[MAX_KEYS];
        static bool ctrl[MAX_KEYS];
        
        static xkb_context* context;
        static xkb_keymap* keymap;
        static xkb_state* state;

        static xkb_keycode_t KeysymToKeycode(const xkb_keysym_t keysym) noexcept;

        static void HandleKeyboardKeymap(void *userData, wl_keyboard *keyboard, 
            uint32 format, int32 fd, uint32 size);

        static void HandleKeyboardKey(void *userData, wl_keyboard *keyboard, 
            uint32 serial, uint32 time, uint32 key, uint32 state);

        static void HandleKeyboardModifiers(void *userData, wl_keyboard *keyboard, 
            uint32 serial, uint32 dep, uint32 lat, uint32 loc, uint32 grp);

        static void SeatHandleCapabilities(void *userData, wl_seat *seat, 
            uint32 caps);

        static void HandleGlobal(void *userData, wl_registry *registry, 
            uint32 id, const char *interface, uint32 version);

    public:
        ~Input() noexcept;
    
        void Initialize(wl_display* display);
        
        bool KeyDown(const uint32 vkcode) noexcept;
        bool KeyUp(const uint32 vkcode) noexcept;
        bool KeyPress(const uint32 vkcode) noexcept;
    };

    inline bool Input::KeyDown(const uint32 vkcode) noexcept
    { return keys[KeysymToKeycode(vkcode)]; }

    inline bool Input::KeyUp(const uint32 vkcode) noexcept
    { return !keys[KeysymToKeycode(vkcode)]; }
}
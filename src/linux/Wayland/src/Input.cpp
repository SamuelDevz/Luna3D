#include "Input.h"
#include "KeyCodes.h"
#include <sys/mman.h>
#include <unistd.h>

namespace Luna
{
    wl_seat* Input::seat = nullptr;
    wl_keyboard* Input::keyboard = nullptr;
    
    bool Input::keys[MAX_KEYS] = {};
    bool Input::ctrl[MAX_KEYS] = {};
    
    xkb_state* Input::state = nullptr;
    xkb_context* Input::context = nullptr;
    xkb_keymap* Input::keymap = nullptr;

    Input::~Input() noexcept
    {
        xkb_state_unref(state);
        xkb_keymap_unref(keymap);
        xkb_context_unref(context);
        wl_keyboard_destroy(keyboard);
        wl_seat_destroy(seat);
    }

    xkb_keycode_t Input::KeysymToKeycode(const xkb_keysym_t keysym) noexcept
    {
        if (!keymap || !state) 
            return 0;

        for (xkb_keycode_t i = 8; i < MAX_KEYS; ++i)
            if (xkb_state_key_get_one_sym(state, i) == keysym) 
                return i;
        return 0;
    }

    void Input::HandleKeyboardKeymap(void *userData, wl_keyboard *keyboard, 
        uint32 format, int32 fd, uint32 size) 
    {
        char *mapStr = static_cast<char*>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
        
        if (mapStr == MAP_FAILED) {
            close(fd);
            return;
        }

        xkb_keymap *newMap = xkb_keymap_new_from_string(
            context, 
            mapStr, 
            XKB_KEYMAP_FORMAT_TEXT_V1, 
            XKB_KEYMAP_COMPILE_NO_FLAGS
        );
        
        munmap(mapStr, size);
        close(fd);

        if (newMap) 
        {
            if (state) xkb_state_unref(state);
            if (keymap) xkb_keymap_unref(keymap);

            keymap = newMap;
            state = xkb_state_new(newMap);
        }
    }

    void Input::HandleKeyboardKey(void *userData, wl_keyboard *keyboard, 
        uint32 serial, uint32 time, uint32 key, uint32 state) 
    {
        const constexpr uint32 XKB_KEYCODE_OFFSET = 8;
        xkb_keycode_t keycode = key + XKB_KEYCODE_OFFSET;
        xkb_keysym_t sym = xkb_state_key_get_one_sym(Input::state, keycode);

        const bool isPressed = (state == WL_KEYBOARD_KEY_STATE_PRESSED);

        if (keycode < MAX_KEYS)
            keys[keycode] = isPressed;
    }

    void Input::HandleKeyboardModifiers(void *userData, wl_keyboard *keyboard, 
        uint32 serial, uint32 dep, uint32 lat, uint32 loc, uint32 grp) 
    {
        if (state)
            xkb_state_update_mask(state, dep, lat, loc, 0, 0, grp);
    }

    void Input::SeatHandleCapabilities(void *userData, wl_seat *seat, uint32 caps) 
    {
        if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD))
            return;
    
        keyboard = wl_seat_get_keyboard(seat);
        static const wl_keyboard_listener keyboardListener = {
            .keymap = HandleKeyboardKeymap,
            .enter = [](void*, wl_keyboard*, uint32, wl_surface*, wl_array*) {},
            .leave = [](void*, wl_keyboard*, uint32, wl_surface*) {},
            .key = HandleKeyboardKey,
            .modifiers = HandleKeyboardModifiers,
            .repeat_info = [](void*, wl_keyboard*, int32, int32) {}
        };
        wl_keyboard_add_listener(keyboard, &keyboardListener, nullptr);
    }

    void Input::HandleGlobal(void *userData, wl_registry *registry, 
        uint32 id, const char *interface, uint32 version)
    {
        const string _interface(interface);
        if (_interface == wl_seat_interface.name)
        {
            seat = static_cast<wl_seat*>(wl_registry_bind(registry, id, &wl_seat_interface, 7));
        }
    }

    void Input::Initialize(wl_display* display)
    {
        context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

        wl_registry* registry = wl_display_get_registry(display);
        static const wl_registry_listener inputListener = {
            .global = HandleGlobal,
            .global_remove = [](void*, wl_registry*, uint32) {}
        };
        wl_registry_add_listener(registry, &inputListener, nullptr);
        wl_display_roundtrip(display);

        static const wl_seat_listener seatListener = {
            .capabilities = SeatHandleCapabilities,
            .name = [](void*, wl_seat*, const char*) {}
        };
        wl_seat_add_listener(seat, &seatListener, nullptr);
        wl_display_roundtrip(display);
    }

    bool Input::KeyPress(const uint32 vkcode) noexcept
    {
        auto keycode = KeysymToKeycode(vkcode);

        if (ctrl[keycode])
        {
            if (KeyDown(vkcode))
            {
                ctrl[keycode] = false;
                return true;
            }
        }
        else if (KeyUp(vkcode))
        {
            ctrl[keycode] = true;
        }

        return false;
    }
}
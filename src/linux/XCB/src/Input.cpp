#include "Input.h"
#include "KeyCodes.h"
#include <locale.h>

namespace Luna
{
    xcb_key_symbols_t* Input::keysyms = nullptr;
    xcb_generic_event_t* Input::event = nullptr;
    xcb_connection_t* Input::connection = nullptr;
    xcb_window_t Input::window = {};

    bool Input::keys[MAX_KEYS] = {};
    bool Input::ctrl[MAX_KEYS] = {};
    string Input::text;
    
    int32 Input::mouseX = 0;
    int32 Input::mouseY = 0;
    int16 Input::mouseWheel = 0;

    xkb_state* Input::state = nullptr;
    xkb_context* Input::context = nullptr;
    xkb_keymap* Input::keymap = nullptr;

    xkb_compose_table* Input::composeTable = nullptr;
    xkb_compose_state* Input::composeState = nullptr;
    
    Input::~Input() noexcept
    {
        xkb_compose_state_unref(composeState);
        xkb_compose_table_unref(composeTable);
        xkb_state_unref(state);
        xkb_keymap_unref(keymap);
        xkb_context_unref(context);
        xcb_key_symbols_free(keysyms);
    }

    xkb_keycode_t Input::KeysymToKeycode(const xkb_keysym_t keysym) const noexcept
    {
        for (xcb_keycode_t kc = xkb_keymap_min_keycode(keymap); kc <= xkb_keymap_max_keycode(keymap); ++kc)
        {
            xcb_keysym_t key = xcb_key_symbols_get_keysym(keysyms, kc, 0);
            if (key == keysym)
                return kc;
        }

        return 0;
    }

    void Input::Initialize(xcb_connection_t* connection, xcb_window_t window, xcb_generic_event_t * event)
    {
        this->connection = connection;
        this->window = window;
        this->event = event;

        keysyms = xcb_key_symbols_alloc(connection);

        xkb_x11_setup_xkb_extension(
            connection,
            XKB_X11_MIN_MAJOR_XKB_VERSION,
            XKB_X11_MIN_MINOR_XKB_VERSION,
            XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        );

        context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
        int32 deviceId = xkb_x11_get_core_keyboard_device_id(connection);
        keymap = xkb_x11_keymap_new_from_device(context, connection, deviceId, XKB_KEYMAP_COMPILE_NO_FLAGS);
        state = xkb_x11_state_new_from_device(keymap, connection, deviceId);

        composeTable = xkb_compose_table_new_from_locale(context, setlocale(LC_CTYPE, nullptr), XKB_COMPOSE_COMPILE_NO_FLAGS);
        composeState = xkb_compose_state_new(composeTable, XKB_COMPOSE_STATE_NO_FLAGS);
    }

    bool Input::XKeyPress(const uint32 vkcode) noexcept
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

    void Input::Read() noexcept
    {
        text.clear();
        Input::Reader(event);
    }

    int16 Input::MouseWheel() noexcept
    {
        int16 val = mouseWheel;
        mouseWheel = 0;
        return val;
    }

    string LookupText(xcb_key_press_event_t * event, xkb_state * state, xkb_compose_state * composeState)
    {
        xkb_keysym_t keysym = xkb_state_key_get_one_sym(state, event->detail);
        if (composeState)
            xkb_compose_state_feed(composeState, keysym);

        enum xkb_compose_status status = composeState ? xkb_compose_state_get_status(composeState) : XKB_COMPOSE_NOTHING;

        char buffer[64]{};
        if (status == XKB_COMPOSE_COMPOSED) 
        {
            xkb_compose_state_get_utf8(composeState, buffer, sizeof(buffer));
            xkb_compose_state_reset(composeState);
        }
        else if (status == XKB_COMPOSE_CANCELLED) 
        {
            xkb_compose_state_reset(composeState);
        } 
        else if (status == XKB_COMPOSE_NOTHING) 
        {
            int size = xkb_state_key_get_utf8(state, event->detail, buffer, sizeof(buffer) - 1);
            if (size > 0) 
                buffer[size] = '\0';
        }

        return string(buffer);
    }

    void Input::Reader(xcb_generic_event_t* event)
    {
        bool read = true;

        while (read)
        {
            event = xcb_wait_for_event(connection);
            
            switch (event->response_type & 0x7f)
            {
                case XCB_MAP_NOTIFY:
                {
                    auto* notify = reinterpret_cast<xcb_mapping_notify_event_t*>(event); 
                    xcb_refresh_keyboard_mapping(keysyms, notify);
                    break;
                }

                case XCB_KEY_PRESS:
                {
                    auto* keyPress = reinterpret_cast<xcb_key_press_event_t*>(event); 
                    xcb_keysym_t key = xcb_key_press_lookup_keysym(keysyms, keyPress, 0);
                    switch (key)
                    {
                    case VK_BACK:
                        if (!text.empty())
                            text.pop_back();
                        break;

                    case VK_TAB:
                        Input::InputProc(event);
                        break;

                    case VK_RETURN:
                        read = false;
                        break;
                    }

                    text += LookupText(keyPress, state, composeState);
                    break;
                }
            }

            Input::InputProc(event);
        }
    }

    void Input::InputProc(xcb_generic_event_t* event)
    {
        switch (event->response_type & 0x7f)
        {
            case XCB_KEY_PRESS:
            {
                auto* keyPress = reinterpret_cast<xcb_key_press_event_t*>(event);
                keys[keyPress->detail] = true;
                break;
            }

            case XCB_KEY_RELEASE:
            {
                auto* keyRelease = reinterpret_cast<xcb_key_release_event_t*>(event);
                keys[keyRelease->detail] = false;
                break;
            }

            case XCB_MOTION_NOTIFY:
            {
                auto* motion = reinterpret_cast<xcb_motion_notify_event_t*>(event);
                mouseX = motion->event_x;
                mouseY = motion->event_y;
                break;
            }

            case XCB_BUTTON_PRESS:
            {
                auto* buttonPress = reinterpret_cast<xcb_button_press_event_t*>(event);
                switch (buttonPress->detail)
                {
                case VK_LBUTTON:
                    keys[VK_LBUTTON] = true;
                    break;

                case VK_MBUTTON:
                    keys[VK_MBUTTON] = true;
                    break;

                case VK_RBUTTON:
                    keys[VK_RBUTTON] = true;
                    break;

                case VK_SCROLL_UP:
                    mouseWheel = 120;
                    break;

                case VK_SCROLL_DOWN:
                    mouseWheel = -120;
                    break;
                }
                break;
            }

            case XCB_BUTTON_RELEASE:
            {
                auto* buttonRelease = reinterpret_cast<xcb_button_release_event_t*>(event);
                switch (buttonRelease->detail)
                {
                case VK_LBUTTON:
                    keys[VK_LBUTTON] = false;
                    break;

                case VK_MBUTTON:
                    keys[VK_MBUTTON] = false;
                    break;

                case VK_RBUTTON:
                    keys[VK_RBUTTON] = false;
                    break;
                }
                break;
            }
        }

        Window::WinProc(event);
    }
}
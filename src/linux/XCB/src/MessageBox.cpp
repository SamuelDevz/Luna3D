#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <string>
#include <string_view>
using std::cerr;
using std::string;
using std::vector;
using std::string_view;

#define XKB_KEY_Escape 0xff1b

/**************************************************************************
* A "small" and "simple" function that creates a message box with an OK  *
* button, using ONLY XCB.                                               *
* The function does not return until the user closes the message box,    *
* using the OK button, the escape key, or the close button what means    *
* that you can't do anything in the mean time(in the same thread).       *
* The code may look very ugly, because I pieced it together from         *
* tutorials and manuals and I use an awfull lot of magic values and      *
* unexplained calculations.                                              *
*                                                                        *
* title: The title of the message box.                                   *
* text:  The contents of the message box. Use '\n' as a line terminator. *
**************************************************************************/

xcb_atom_t GetAtom(xcb_connection_t* connection, const string_view atom)
{
    auto cookie = xcb_intern_atom(connection, 0, atom.size(), atom.data());
    auto atomReply = xcb_intern_atom_reply(connection, cookie, nullptr);
    xcb_atom_t result = atomReply->atom;

    delete atomReply;
    
    return result;
}

void MessageBox(const char * title, const char * text)
{
    xcb_connection_t *connection = xcb_connect(nullptr, nullptr);
    if (!connection) 
    {
        cerr << "Failed to connect to X server\n";
        return;
    }

    xcb_screen_t *screen = nullptr;
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(connection));
    if (iter.rem)
        screen = iter.data;

    if (!screen) 
    {
        cerr << "Failed to get screen\n";
        xcb_disconnect(connection);
        return;
    }

    uint32_t black = screen->black_pixel;
    uint32_t white = screen->white_pixel;

    /* Window creation */
    xcb_window_t window = xcb_generate_id(connection);
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[2] = { white, XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                                    XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION};

    xcb_create_window(connection,
        XCB_COPY_FROM_PARENT,
        window,
        screen->root,
        0, 0,
        100, 100,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        mask,
        values);

    /* Set window title */
    xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window,
        XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(title), title);

    /*  WM_DELETE_WINDOW protocol */
    xcb_atom_t wm_delete_atom = GetAtom(connection, "WM_DELETE_WINDOW");
    xcb_atom_t wm_protocols_atom = GetAtom(connection, "WM_PROTOCOLS");
    xcb_change_property(
        connection, 
        XCB_PROP_MODE_REPLACE, 
        window, 
        wm_protocols_atom, 
        XCB_ATOM_ATOM, 
        32, 
        1, 
        &wm_delete_atom
    );

    /* Setting _NET_WM_WINDOW_TYPE_DIALOG */
    xcb_atom_t window_type_atom = GetAtom(connection, "_NET_WM_WINDOW_TYPE");
    xcb_atom_t dialog_atom = GetAtom(connection, "_NET_WM_WINDOW_TYPE_DIALOG");
    xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window,
                        window_type_atom, XCB_ATOM_ATOM, 32, 1, &dialog_atom);

    /* Create graphics context */
    xcb_gcontext_t gc = xcb_generate_id(connection);
    uint32_t gc_mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
    uint32_t gc_values[2] = { black, white };
    xcb_create_gc(connection, gc, window, gc_mask, gc_values);

    /* Split the text into lines */
    vector<string> lines;
    string text_str(text);
    size_t start = 0, end = 0;
    string token;
    while ((end = text_str.find('\n', start)) != string::npos) 
    {
        token = text_str.substr(start, end - start);
        lines.push_back(token);
        start = end + 1;
    }
    lines.push_back(text_str.substr(start));

    /* Font and text extents (replace with a proper font loading and handling) */
    xcb_font_t font = xcb_generate_id(connection);
    xcb_open_font(connection, font, strlen("fixed"), "fixed");

    xcb_query_font_reply_t *font_reply = xcb_query_font_reply(connection, xcb_query_font(connection, font), nullptr);
    if (!font_reply) 
    {
        cerr << "Failed to load font\n";
        xcb_disconnect(connection);
        return;
    }

    int ascent = font_reply->font_ascent;
    int descent = font_reply->font_descent;
    int height = ascent + descent;

    int length = 0;
    for (const auto& line : lines)
        length = std::max(length, (int)line.length() * 8); // crude approximation

    /* Window geometry */
    const int X = (screen->width_in_pixels / 2) - (length / 2) - 10;
    const int Y = (screen->height_in_pixels / 2) - (height / 2) - 10;
    const int W = length + 20;
    const int H = (int)lines.size() * height + height + 40;

    xcb_configure_window_value_list_t config_values;
    config_values.x = X;
    config_values.y = Y;
    config_values.width = W;
    config_values.height = H;

    uint32_t config_mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
    xcb_configure_window(connection, window, config_mask, &config_values);

    /* OK button geometry (crude approximation) */
    constexpr int okWidth = 30;
    const int okHeight = height;
    const int okX1 = W / 2 - okWidth / 2 - 15;
    const int okY1 = (int)lines.size() * height + 20;
    const int okX2 = W / 2 + okWidth / 2 + 15;
    const int okY2 = okY1 + okHeight;
    const int okBaseX = okX1 + 15;
    const int okBaseY = okY1 + ascent;

    /* Map the window */
    xcb_map_window(connection, window);
    xcb_flush(connection);

    /* Event loop */
    bool run = true, buttonFocus = false;
    while (run)
    {
        xcb_generic_event_t *event = xcb_wait_for_event(connection);
        if (!event)
            break;

        switch (event->response_type & 0x7f)
        {
            case XCB_MOTION_NOTIFY:
            {
                xcb_motion_notify_event_t *motion = (xcb_motion_notify_event_t *)event;
                if (motion->event_x >= okX1 && motion->event_x <= okX2 && motion->event_y >= okY1 && motion->event_y <= okY2) 
                {
                    if (!buttonFocus)
                        event->response_type = XCB_EXPOSE;
                    buttonFocus = true;
                } 
                else 
                {
                    if (buttonFocus)
                        event->response_type = XCB_EXPOSE;
                    buttonFocus = false;
                }
                break;
            }
            case XCB_BUTTON_PRESS:
            case XCB_BUTTON_RELEASE:
            {
                xcb_button_press_event_t *button = (xcb_button_press_event_t *)event;
                if (button->detail != XCB_BUTTON_INDEX_1)
                    break;

                if (buttonFocus)
                    run = false;
                break;
            }
            case XCB_EXPOSE:
            {
                // Draw text lines
                int y = 10 + ascent;
                for (const auto& line : lines) 
                {
                    xcb_image_text_8(connection, line.length(), window, gc, 10, y, line.c_str());
                    y += height;
                }

                // Draw OK button
                if (buttonFocus) 
                {
                    xcb_rectangle_t rect = { (int16_t)okX1, (int16_t)okY1, (uint16_t)(okX2 - okX1), (uint16_t)(okY2 - okY1) };
                    xcb_poly_fill_rectangle(connection, window, gc, 1, &rect);
                    xcb_change_gc(connection, gc, XCB_GC_FOREGROUND, &white);
                }
                else
                {
                    xcb_change_gc(connection, gc, XCB_GC_FOREGROUND, &black);
                    xcb_image_text_8(connection, 2, window, gc, okBaseX, okBaseY, "OK");
                }
                xcb_image_text_8(connection, 2, window, gc, okBaseX, okBaseY, "OK");

                xcb_flush(connection);
                break;
            }
            case XCB_KEY_RELEASE:
            {
                xcb_key_release_event_t *key = (xcb_key_release_event_t *)event;
                xcb_key_symbols_t  *keysyms = xcb_key_symbols_alloc(connection);
                xcb_keysym_t keysym = 0;
                
                if (keysyms)
                {
                    keysym = xcb_key_symbols_get_keysym(keysyms, key->detail, 0);
                    xcb_key_symbols_free(keysyms);
                }
                
                if (keysym == XKB_KEY_Escape)
                    run = false;
                break;
            }
            case XCB_CLIENT_MESSAGE:
            {
                xcb_client_message_event_t *cm = (xcb_client_message_event_t *)event;
                if (cm->data.data32[0] == wm_delete_atom)
                    run = false;
                break;
            }
        }
        free(event);
    }

    /* Cleanup */
    xcb_close_font(connection, font);
    xcb_free_gc(connection, gc);
    xcb_destroy_window(connection, window);
    xcb_disconnect(connection);
}
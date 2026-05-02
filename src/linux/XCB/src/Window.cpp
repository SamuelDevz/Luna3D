#include "Window.h"
#include <X11/Xlib-xcb.h>
#include <xcb/xcb_icccm.h>
#include <unistd.h>
#include <png.h>

namespace Luna
{
    void (*Window::inFocus)() = nullptr;
    void (*Window::lostFocus)() = nullptr;

    Window::Window() noexcept 
        : windowHandle{}, 
        windowPosX{}, 
        windowPosY{}
    {
        windowDisplay = XOpenDisplay(nullptr);
        windowConnection = XGetXCBConnection(windowDisplay);
        windowCursor = XcursorFilenameLoadCursor(windowDisplay, "left_ptr");
        windowHandle = xcb_generate_id(windowConnection);
        windowScreen = xcb_setup_roots_iterator(xcb_get_setup(windowConnection)).data;
        windowWidth = windowScreen->width_in_pixels;
        windowHeight = windowScreen->height_in_pixels;
        windowColor = windowScreen->white_pixel;
        windowTitle = string("Window Game");
        windowMode = FULLSCREEN;
        windowCenterX = windowWidth / 2;
        windowCenterY = windowHeight / 2;

        XSetEventQueueOwner(windowDisplay, XCBOwnsEventQueue);
    }

    Window::~Window() noexcept
    {
        xcb_unmap_window(windowConnection, windowHandle);
        xcb_destroy_window(windowConnection, windowHandle);
        xcb_disconnect(windowConnection);
    }

    uint32 Window::GetColor(const string && hexColor) noexcept
    {
        const uint32 rgb = stoul(hexColor.substr(1), nullptr, 16);
    
        uint16 r = ((rgb >> 16) & 0xFF) * 0x0101;
        uint16 g = ((rgb >> 8) & 0xFF) * 0x0101;
        uint16 b = (rgb & 0xFF) * 0x0101;
    
        auto cookie = xcb_alloc_color(windowConnection, windowScreen->default_colormap, r, g, b);
        auto reply = xcb_alloc_color_reply(windowConnection, cookie, nullptr);
    
        if (!reply) 
            return 0;
    
        const uint32 pixel = reply->pixel;
        free(reply);
    
        return pixel;
    }

    void Window::Size(const uint32 width, const uint32 height) noexcept
    { 
        windowWidth = width; 
        windowHeight = height;

        windowCenterX = windowWidth / 2;
        windowCenterY = windowHeight / 2;

        windowPosX = (windowScreen->width_in_pixels - windowWidth) / 2;
        windowPosY = (windowScreen->height_in_pixels - windowHeight) / 2;
    }

    void Window::Close() noexcept
    {
        xcb_client_message_event_t event{};
        event.response_type = XCB_CLIENT_MESSAGE;
        event.format = 32;
        event.window = windowHandle;
        event.type = wmProtocols;
        event.data.data32[0] = wmDeleteWindow;
        event.data.data32[1] = XCB_CURRENT_TIME;

        xcb_send_event(
            windowConnection, 
            false, 
            windowHandle, 
            XCB_EVENT_MASK_NO_EVENT, 
            reinterpret_cast<const char*>(&event)
        );
        xcb_flush(windowConnection); 
    }

    static xcb_atom_t GetAtom(xcb_connection_t* connection, const string_view atom)
    {
        auto cookie = xcb_intern_atom(connection, 0, atom.size(), atom.data());
        auto atomReply = xcb_intern_atom_reply(connection, cookie, nullptr);
        const xcb_atom_t result = atomReply->atom;
        delete atomReply;
        return result;
    }

    static bool LoadPNG(const string_view filename, uint8 ** imageData, int32 & width, int32 & height)
    {
        FILE *fp = fopen(filename.data(), "rb");
        if (!fp)
            return false;
    
        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png) 
        {
            fclose(fp);
            return false;
        }
    
        png_infop info = png_create_info_struct(png);
        if (!info) 
        {
            png_destroy_read_struct(&png, nullptr, nullptr);
            fclose(fp);
            return false;
        }
    
        if (setjmp(png_jmpbuf(png))) 
        {
            png_destroy_read_struct(&png, &info, nullptr);
            fclose(fp);
            return false;
        }
    
        png_init_io(png, fp);
        png_read_info(png, info);
    
        width = png_get_image_width(png, info);
        height = png_get_image_height(png, info);
        png_byte bitDepth = png_get_bit_depth(png, info);
        png_byte colorType = png_get_color_type(png, info);
    
        if (colorType == PNG_COLOR_TYPE_PALETTE) 
            png_set_palette_to_rgb(png);

        if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8) 
            png_set_expand_gray_1_2_4_to_8(png);
        
        if (png_get_valid(png, info, PNG_INFO_tRNS))
            png_set_tRNS_to_alpha(png);

        if (bitDepth == 16)
            png_set_strip_16(png);
    
        png_set_expand(png);
        png_read_update_info(png, info);
    
        *imageData = new uint8[png_get_rowbytes(png, info) * height];
        png_bytep* rowPointers = new png_bytep[height];
        for (size_t y = 0; y < height; ++y)
            rowPointers[y] = *imageData + y * png_get_rowbytes(png, info);
    
        png_read_image(png, rowPointers);
        png_destroy_read_struct(&png, &info, nullptr);
        delete[] rowPointers;
        fclose(fp);

        return true;
    }

    static void SetIcon(Display * display, xcb_connection_t* connection, xcb_window_t window, const string_view filename) 
    {
        int32 width, height;
        uint8 * dataImage = nullptr;
        LoadPNG(filename.data(), &dataImage, width, height);
    
        int32 longCount = 2 + width * height;
        uint64 * icon = new uint64[longCount];
        uint64 * target = icon;
    
        *target++ = width;
        *target++ = height;

        for (size_t i = 0; i < width * height; ++i) 
        {
            *target++ = 
            (dataImage[i * 4 + 2]) |
            (dataImage[i * 4 + 1] << 8) |
            (dataImage[i * 4 + 0] << 16) |
            (dataImage[i * 4 + 3] << 24);
        }
        
        /*
        // I don't know the why xcb_change_property doesn't work here!
        xcb_atom_t _NET_WM_ICON = GetAtom(connection, "_NET_WM_ICON");
        xcb_change_property(
            connection, 
            XCB_PROP_MODE_REPLACE, 
            window,
            _NET_WM_ICON,
            XCB_ATOM_CARDINAL, 
            32,
            longCount, 
            icon
        );
        */

        auto _NET_WM_ICON = XInternAtom(display, "_NET_WM_ICON", false);
        XChangeProperty(
            display, 
            window,
            _NET_WM_ICON,
            XCB_ATOM_CARDINAL, 
            32,
            XCB_PROP_MODE_REPLACE,
            reinterpret_cast<uint8*>(icon),
            longCount
        );
        
        delete[] icon;
        delete[] dataImage;
        xcb_flush(connection);
    }

    static void Fullscreen(xcb_connection_t* connection, xcb_window_t window)
    {
        xcb_atom_t _NET_WM_STATE = GetAtom(connection, "_NET_WM_STATE");
        xcb_atom_t _NET_WM_STATE_FULLSCREEN = GetAtom(connection, "_NET_WM_STATE_FULLSCREEN");
    
        xcb_change_property(
            connection, 
            XCB_PROP_MODE_REPLACE, 
            window,
            _NET_WM_STATE, 
            XCB_ATOM_ATOM, 
            32, 
            1, 
            &_NET_WM_STATE_FULLSCREEN
        );
    }

    static void Borderless(xcb_connection_t* connection, xcb_window_t window) 
    {
        struct
        {
            uint32 flags;
            uint32 functions;
            uint32 decorations;
            int32 input_mode;
            uint32 status;
        } hints{};

        hints.flags = 2;

        xcb_atom_t _MOTIF_WM_HINTS = GetAtom(connection, "_MOTIF_WM_HINTS");
        xcb_change_property(
            connection,
            XCB_PROP_MODE_REPLACE,
            window,
            _MOTIF_WM_HINTS,
            _MOTIF_WM_HINTS,
            32,
            5,
            &hints
        );
    }

    void SetAtoms(xcb_connection_t* connection, xcb_window_t window, uint32 windowMode)
    {
        if(windowMode == BORDERLESS)
            Borderless(connection, window);
        else if(windowMode == FULLSCREEN)
            Fullscreen(connection, window);

        auto pid = getpid();
        xcb_atom_t _NET_WM_PID = GetAtom(connection, "_NET_WM_PID");
        xcb_change_property(
            connection, 
            XCB_PROP_MODE_REPLACE, 
            window,
            _NET_WM_PID, 
            XCB_ATOM_CARDINAL, 
            32, 
            1,
            &pid
        );

        xcb_atom_t _NET_WM_WINDOW_TYPE = GetAtom(connection, "_NET_WM_WINDOW_TYPE");
        xcb_atom_t _NET_WM_WINDOW_TYPE_NORMAL = GetAtom(connection, "_NET_WM_WINDOW_TYPE_NORMAL");
        xcb_change_property(
            connection, 
            XCB_PROP_MODE_REPLACE, 
            window,
            _NET_WM_WINDOW_TYPE, 
            XCB_ATOM_CARDINAL, 
            32, 
            1, 
            &_NET_WM_WINDOW_TYPE_NORMAL
        );

        uint64 compositor = 1;
        xcb_atom_t _NET_WM_BYPASS_COMPOSITOR = GetAtom(connection, "_NET_WM_BYPASS_COMPOSITOR");
        xcb_change_property(
            connection, 
            XCB_PROP_MODE_REPLACE, 
            window, 
            _NET_WM_BYPASS_COMPOSITOR,
            XCB_ATOM_CARDINAL, 
            32, 
            1, 
            &compositor
        );
    }

    bool Window::Create() noexcept
    {
        if(xcb_connection_has_error(windowConnection))
            return false;

        xcb_create_window_value_list_t attributes{};
        attributes.background_pixel = windowColor;
        attributes.event_mask = XCB_EVENT_MASK_EXPOSURE 
            | XCB_EVENT_MASK_BUTTON_PRESS   
            | XCB_EVENT_MASK_BUTTON_RELEASE 
            | XCB_EVENT_MASK_POINTER_MOTION 
            | XCB_EVENT_MASK_ENTER_WINDOW   
            | XCB_EVENT_MASK_LEAVE_WINDOW   
            | XCB_EVENT_MASK_KEY_PRESS      
            | XCB_EVENT_MASK_KEY_RELEASE    
            | XCB_EVENT_MASK_FOCUS_CHANGE;

        const uint32 mask = XCB_CW_BACK_PIXEL 
            | XCB_CW_EVENT_MASK 
            | XCB_CW_COLORMAP 
            | XCB_CW_CURSOR;
        
        xcb_create_window_aux(
            windowConnection,
            XCB_COPY_FROM_PARENT,
            windowHandle,
            windowScreen->root,
            static_cast<int16>(windowPosX), static_cast<int16>(windowPosY),
            static_cast<uint16>(windowWidth), static_cast<uint16>(windowHeight),
            0,
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            XCB_COPY_FROM_PARENT,
            mask,
            &attributes
        );

        if(!windowHandle)
            return false;

        xcb_icccm_set_wm_name(
            windowConnection,
            windowHandle, 
            XCB_ATOM_STRING, 
            8,
            windowTitle.size(), 
            windowTitle.c_str()
        );

        xcb_size_hints_t sizeHints{};
        xcb_icccm_size_hints_set_min_size(&sizeHints, windowWidth, windowHeight);
        xcb_icccm_size_hints_set_max_size(&sizeHints, windowWidth, windowHeight);
        xcb_icccm_set_wm_size_hints(windowConnection, windowHandle, XCB_ATOM_WM_NORMAL_HINTS, &sizeHints);

        wmDeleteWindow = GetAtom(windowConnection, "WM_DELETE_WINDOW");
        wmProtocols = GetAtom(windowConnection, "WM_PROTOCOLS");
        xcb_icccm_set_wm_protocols(windowConnection, windowHandle, wmProtocols, 1, &wmDeleteWindow);

        SetAtoms(windowConnection, windowHandle, windowMode);
        
        XDefineCursor(windowDisplay, windowHandle, windowCursor);
        if(!windowIcon.empty())
            SetIcon(windowDisplay, windowConnection, windowHandle, windowIcon);
        
        xcb_map_window (windowConnection, windowHandle);
        
        const uint32 valueMask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
        const int32 coord[] = { windowPosX, windowPosY };
        xcb_configure_window(windowConnection, windowHandle, valueMask, coord);

        xcb_flush(windowConnection);

        return true;
    }

    void Window::WinProc(const xcb_generic_event_t * const event)
    {
        switch(event->response_type & 0x7f)
        {
        case XCB_FOCUS_OUT:
            if (lostFocus)
                lostFocus();
            break;

        case XCB_FOCUS_IN:
            if (inFocus)
                inFocus();
            break;
        }
    }
}
#include "Window.h"
#include <xcb/xcb_icccm.h>
#include <unistd.h>
#include <cstdlib>
#include <png.h>

namespace Luna
{
    void (*Window::inFocus)() = nullptr;
    void (*Window::lostFocus)() = nullptr;

    Window::Window() noexcept 
        : window{}, 
        windowPosX{}, 
        windowPosY{}
    {
        XInitThreads();

        display = XOpenDisplay(nullptr);
        connection = XGetXCBConnection(display);
        windowCursor = XcursorFilenameLoadCursor(display, "left_ptr");
        window = xcb_generate_id(connection);
        screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
        windowWidth = screen->width_in_pixels;
        windowHeight = screen->height_in_pixels;
        windowColor = screen->white_pixel;
        windowTitle = string("Window Game");
        windowMode = FULLSCREEN;
        windowCenterX = windowWidth / 2;
        windowCenterY = windowHeight / 2;

        XSetEventQueueOwner(display, XCBOwnsEventQueue);
    }

    Window::~Window() noexcept
    {
        xcb_unmap_window(connection, window);
        xcb_destroy_window(connection, window);
        xcb_disconnect(connection);
    }

    uint32 Window::GetColor(const string hexColor) noexcept
    {
        const string redStr = hexColor.substr(1, 2);
        const string greenStr = hexColor.substr(3, 2);
        const string blueStr = hexColor.substr(5, 2);

        auto redLong = strtol(redStr.c_str(), nullptr, 16);
        auto greenLong = strtol(greenStr.c_str(), nullptr, 16);
        auto blueLong = strtol(blueStr.c_str(), nullptr, 16);

        const constexpr uint16 colorScaleFactor = 257; // 65535 / 255

        uint16 red = static_cast<uint16>(redLong * colorScaleFactor);
        uint16 green = static_cast<uint16>(greenLong * colorScaleFactor);
        uint16 blue = static_cast<uint16>(blueLong * colorScaleFactor);

        xcb_colormap_t colormap = screen->default_colormap;
        auto cookie = xcb_alloc_color(connection, colormap, red, green, blue);
        auto reply = xcb_alloc_color_reply(connection, cookie, nullptr);
        
        uint32 pixel = reply->pixel;
        
        delete reply;

        return pixel;
    }

    void Window::Size(const uint32 width, const uint32 height) noexcept
    { 
        windowWidth = width; 
        windowHeight = height;

        windowCenterX = windowWidth / 2;
        windowCenterY = windowHeight / 2;

        windowPosX = (screen->width_in_pixels - windowWidth) / 2;
        windowPosY = (screen->height_in_pixels - windowHeight) / 2;
    }

    void Window::Close() noexcept
    {
        xcb_client_message_event_t event{};
        event.response_type = XCB_CLIENT_MESSAGE;
        event.format = 32;
        event.window = window;
        event.type = wmProtocols;
        event.data.data32[0] = wmDeleteWindow;
        event.data.data32[1] = XCB_CURRENT_TIME;

        xcb_send_event(
            connection, 
            false, 
            window, 
            XCB_EVENT_MASK_NO_EVENT, 
            reinterpret_cast<const char*>(&event)
        );
        xcb_flush(connection); 
    }

    xcb_atom_t GetAtom(xcb_connection_t* connection, const string_view atom)
    {
        auto cookie = xcb_intern_atom(connection, 0, atom.size(), atom.data());
        auto atomReply = xcb_intern_atom_reply(connection, cookie, nullptr);
        xcb_atom_t result = atomReply->atom;

        delete atomReply;
        
        return result;
    }

    bool LoadPNG(const string_view filename, unsigned char ** imageData, int32 & width, int32 & height)
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
    
        *imageData = new unsigned char[png_get_rowbytes(png, info) * height];
        png_bytep* rowPointers = new png_bytep[height];
        for (int y = 0; y < height; ++y)
            rowPointers[y] = *imageData + y * png_get_rowbytes(png, info);
    
        png_read_image(png, rowPointers);
        png_destroy_read_struct(&png, &info, nullptr);
        delete[] rowPointers;
        fclose(fp);

        return true;
    }

    void SetIcon(Display * display, xcb_connection_t* connection, xcb_window_t window, const string_view filename) 
    {
        int width, height;
        unsigned char * dataImage;
        LoadPNG(filename.data(), &dataImage, width, height);
    
        int longCount = 2 + width * height;
    
        unsigned long* icon = new unsigned long[longCount];
        unsigned long* target = icon;
    
        *target++ = width;
        *target++ = height;
    
        for (int i = 0; i < width * height; ++i) 
        {
            *target++ = 
                (dataImage[i * 4 + 2]) |
                (dataImage[i * 4 + 1] << 8) |
                (dataImage[i * 4 + 0] << 16) |
                (dataImage[i * 4 + 3] << 24);
        }
        
        /*
        // I don't know the why xcb_change_property doesn't work here!
        auto netWmIcon = GetAtom(connection, "_NET_WM_ICON");
        xcb_change_property(
            connection, 
            XCB_PROP_MODE_REPLACE, 
            window,
            netWmIcon,
            XCB_ATOM_CARDINAL, 
            32,
            longCount, 
            icon
        );
        */

        auto netWmIcon = XInternAtom(display, "_NET_WM_ICON", false);
        XChangeProperty(
            display, 
            window,
            netWmIcon,
            XCB_ATOM_CARDINAL, 
            32,
            XCB_PROP_MODE_REPLACE,
            (unsigned char*) icon,
            longCount
        );
        
        delete[] icon;
        delete[] dataImage;
        xcb_flush(connection);
    }

    void Fullscreen(xcb_connection_t* connection, xcb_window_t window)
    {
        xcb_atom_t netWmState = GetAtom(connection, "_NET_WM_STATE");
        xcb_atom_t netWmStateFullscreen = GetAtom(connection, "_NET_WM_STATE_FULLSCREEN");
    
        xcb_change_property(
            connection, 
            XCB_PROP_MODE_REPLACE, 
            window,
            netWmState, 
            XCB_ATOM_ATOM, 
            32, 
            1, 
            &netWmStateFullscreen
        );
    }

    void Borderless(xcb_connection_t* connection, xcb_window_t window) 
    {
        struct
        {
            uint32   flags;
            uint32   functions;
            uint32   decorations;
            int32    input_mode;
            uint32   status;
        } hints{};

        hints.flags = 2;

        xcb_atom_t motifWmHints = GetAtom(connection, "_MOTIF_WM_HINTS");
        xcb_change_property(
            connection,
            XCB_PROP_MODE_REPLACE,
            window,
            motifWmHints,
            motifWmHints,
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
        xcb_atom_t newWmPid = GetAtom(connection, "_NET_WM_PID");
        xcb_change_property(
            connection, 
            XCB_PROP_MODE_REPLACE, 
            window,
            newWmPid, 
            XCB_ATOM_CARDINAL, 
            32, 
            1,
            &pid
        );

        Atom newWmWindowType = GetAtom(connection, "_NET_WM_WINDOW_TYPE");
        Atom newWmWindowTypeNormal = GetAtom(connection, "_NET_WM_WINDOW_TYPE_NORMAL");
        xcb_change_property(
            connection, 
            XCB_PROP_MODE_REPLACE, 
            window,
            newWmWindowType, 
            XCB_ATOM_CARDINAL, 
            32, 
            1, 
            &newWmWindowTypeNormal
        );

        long compositor = 1;
        Atom newWmBypassCompositor = GetAtom(connection, "_NET_WM_BYPASS_COMPOSITOR");
        xcb_change_property(
            connection, 
            XCB_PROP_MODE_REPLACE, 
            window, 
            newWmBypassCompositor,
            XCB_ATOM_CARDINAL, 
            32, 
            1, 
            &compositor
        );
    }

    bool Window::Create() noexcept
    {
        if(xcb_connection_has_error(connection))
            return false;

        uint32 mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

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

        xcb_create_window_aux(
            connection,
            XCB_COPY_FROM_PARENT,
            window,
            screen->root,
            static_cast<int16>(windowPosX), static_cast<int16>(windowPosY),
            static_cast<uint16>(windowWidth), static_cast<uint16>(windowHeight),
            0,
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            XCB_COPY_FROM_PARENT,
            mask,
            &attributes
        );

        if(!window)
            return false;

        xcb_icccm_set_wm_name(
            connection,
            window, 
            XCB_ATOM_STRING, 
            8,
            windowTitle.size(), 
            windowTitle.c_str()
        );

        xcb_size_hints_t sizeHints{};
        xcb_icccm_size_hints_set_min_size(&sizeHints, windowWidth, windowHeight);
        xcb_icccm_size_hints_set_max_size(&sizeHints, windowWidth, windowHeight);
        xcb_icccm_set_wm_size_hints(connection, window, XCB_ATOM_WM_NORMAL_HINTS, &sizeHints);

        wmDeleteWindow = GetAtom(connection, "WM_DELETE_WINDOW");
        wmProtocols = GetAtom(connection, "WM_PROTOCOLS");
        
        xcb_icccm_set_wm_protocols(connection, window, wmProtocols, 1, &wmDeleteWindow);

        SetAtoms(connection, window, windowMode);
        
        XDefineCursor(display, window, windowCursor);

        if(!windowIcon.empty())
            SetIcon(display, connection, window, windowIcon);
        
        xcb_map_window (connection, window);
        
        uint32 valueMask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
        int32 coord[] = { windowPosX, windowPosY };
        xcb_configure_window(connection, window, valueMask, coord);

        xcb_flush(connection);

        return true;
    }
}
#include "Window.h"
#include <unistd.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <png.h>

namespace Luna
{
    void (*Window::inFocus)() = nullptr;
    void (*Window::lostFocus)() = nullptr;

    Window::Window() noexcept
        : windowHandle{},
        windowPosX{},
        windowPosY{},
        windowIcon{}
    {
        XInitThreads();

        windowDisplay = XOpenDisplay(nullptr);
        windowScreen = DefaultScreenOfDisplay(windowDisplay);
        windowWidth = windowScreen->width;
        windowHeight = windowScreen->height;
        windowCursor = XCreateFontCursor(windowDisplay, XC_left_ptr);
        windowColor.pixel = WhitePixel(windowDisplay, DefaultScreen(windowDisplay));
        windowTitle = string("Windows Game");
        windowMode = FULLSCREEN;
        windowCenterX = windowWidth / 2;
        windowCenterY = windowHeight / 2;
    }

    Window::~Window() noexcept
    {
        XFreeColors(windowDisplay, DefaultColormap(windowDisplay, DefaultScreen(windowDisplay)), &windowColor.pixel, 1, 0);
        XFreeCursor(windowDisplay, windowCursor);
        XUnmapWindow(windowDisplay, windowHandle);
        XDestroyWindow(windowDisplay, windowHandle);
        XCloseDisplay(windowDisplay);
    }

    uint32 Window::GetColor(const char * color) noexcept
    {
        XColor hex{};
        XParseColor(windowDisplay, DefaultColormap(windowDisplay, 0), color, &hex);
        XAllocColor(windowDisplay, DefaultColormap(windowDisplay, 0), &hex);
        return hex.pixel;
    }

    void Window::Size(const uint32 width, const uint32 height) noexcept
    {
        windowWidth = width;
        windowHeight = height;

        windowCenterX = windowWidth / 2;
        windowCenterY = windowHeight / 2;

        windowPosX = (windowScreen->width - windowWidth) / 2;
        windowPosY = (windowScreen->height - windowHeight) / 2;
    }

    static void SendEventToWM(Display * display, XWindow window, Atom type,
        uint64 eventMask, const uint64 a, const uint64 b, 
        const uint64 c = 0, const uint64 d = 0, const uint64 e = 0)
    {
        XEvent event{};
        event.type = ClientMessage;
        event.xclient.window = window;
        event.xclient.format = 32;
        event.xclient.message_type = type;
        event.xclient.data.l[0] = a;
        event.xclient.data.l[1] = b;
        event.xclient.data.l[2] = c;
        event.xclient.data.l[3] = d;
        event.xclient.data.l[4] = e;

        XSendEvent(display, window, false, eventMask, &event);
        XFlush(display);
    }

    static void X11ChangeProperty(Display * display, XWindow window, Atom property,
        Atom type, const uint8*	data, int32 nelements)
    {
        XChangeProperty(
            display,
            window,
            property,
            type,
            32,
            PropModeReplace,
            data,
            nelements
        );
    }

    void Window::Close() noexcept
    {
        SendEventToWM(
            windowDisplay,
            windowHandle,
            wmProtocols,
            NoEventMask,
            wmDeleteWindow,
            CurrentTime
        );
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
        const png_byte bitDepth = png_get_bit_depth(png, info);
        const png_byte colorType = png_get_color_type(png, info);

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

    static void SetIcon(Display * display, XWindow window, const string_view filename)
    {
        int32 width, height;
        uint8 * dataImage;
        LoadPNG(filename.data(), &dataImage, width, height);

        int32 longCount = 2 + width * height;

        uint64* icon = new uint64[longCount * sizeof(uint64)];
        uint64* target = icon;

        *target++ = width;
        *target++ = height;

        for (size_t i = 0; i < width * height; ++i)
        {
            *target++ =
            ((dataImage[i * 4 + 2])) |
            ((dataImage[i * 4 + 1]) << 8) |
            ((dataImage[i * 4 + 0]) << 16) |
            ((dataImage[i * 4 + 3]) << 24);
        }

        Atom _NET_WM_ICON = XInternAtom(display, "_NET_WM_ICON", false);
        X11ChangeProperty(
            display,
            window,
            _NET_WM_ICON,
            XA_CARDINAL,
            reinterpret_cast<uint8*>(icon),
            longCount
        );

        delete[] icon;
    }

    static void Fullscreen(Display *display, XWindow window)
    {
        Atom _NET_WM_STATE = XInternAtom(display, "_NET_WM_STATE", false);
        Atom _NET_WM_STATE_FULLSCREEN = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", false);

        XSizeHints sizehints{};
        int64 flags{};
        XGetWMNormalHints(display, window, &sizehints, reinterpret_cast<long int*>(&flags));

        sizehints.flags &= ~(PMinSize | PMaxSize);
        XSetWMNormalHints(display, window, &sizehints);

        enum
        {
            _NET_WM_STATE_REMOVE = 0,
            _NET_WM_STATE_ADD,
            _NET_WM_STATE_TOGGLE,
        };

        SendEventToWM(
            display,
            RootWindow(display, 0),
            _NET_WM_STATE,
            SubstructureNotifyMask | SubstructureRedirectMask,
            _NET_WM_STATE_ADD,
            _NET_WM_STATE_FULLSCREEN
        );
    }

    static void Borderless(Display * display, XWindow window)
    {
        struct
        {
            uint64 flags;
            uint64 functions;
            uint64 decorations;
            int64 input_mode;
            uint64 status;
        } hints = {};

        enum
        {
            MWM_DECOR_ALL = 1,
            MWM_HINTS_DECORATIONS = 2,
        };

        hints.flags = MWM_HINTS_DECORATIONS;

        Atom _MOTIF_WM_HINTS = XInternAtom(display, "_MOTIF_WM_HINTS", false);
        X11ChangeProperty(
            display,
            window,
            _MOTIF_WM_HINTS,
            _MOTIF_WM_HINTS,
            reinterpret_cast<uint8*>(&hints),
            sizeof(hints) / sizeof(int64)
        );
    }

    static void SetAtoms(Display * display, XWindow window, uint32 windowMode)
    {
        if(windowMode == BORDERLESS)
            Borderless(display, window);
        else if(windowMode == FULLSCREEN)
            Fullscreen(display, window);

        auto pid = getpid();
        Atom _NET_WM_PID = XInternAtom(display, "_NET_WM_PID", false);
        X11ChangeProperty(
            display,
            window,
            _NET_WM_PID,
            XA_CARDINAL,
            reinterpret_cast<uint8*>(&pid),
            1
        );

        Atom _NET_WM_WINDOW_TYPE = XInternAtom(display, "_NET_WM_WINDOW_TYPE", false);
        Atom _NET_WM_WINDOW_TYPE_NORMAL = XInternAtom(display, "_NET_WM_WINDOW_TYPE_NORMAL", false);
        X11ChangeProperty(
            display,
            window,
            _NET_WM_WINDOW_TYPE,
            XA_ATOM,
            reinterpret_cast<uint8*>(&_NET_WM_WINDOW_TYPE_NORMAL),
            1
        );

        uint8 compositor = 1;
        Atom _NET_WM_BYPASS_COMPOSITOR = XInternAtom(display, "_NET_WM_BYPASS_COMPOSITOR", false);
        X11ChangeProperty(
            display,
            window,
            _NET_WM_BYPASS_COMPOSITOR,
            XA_CARDINAL,
            &compositor,
            1
        );
    }

    bool Window::Create() noexcept
    {
        if(!windowDisplay)
            return false;

        XSetWindowAttributes attributes{};
        attributes.background_pixel = windowColor.pixel;
        attributes.event_mask = ExposureMask
            | PointerMotionMask
            | ButtonPressMask
            | ButtonReleaseMask
            | KeyPressMask
            | FocusChangeMask
            | ButtonMotionMask;
        uint32 valueMask = CWBackPixel | CWBorderPixel | CWEventMask;

        windowHandle = XCreateWindow(
            windowDisplay,
            RootWindow(windowDisplay, 0),
            windowPosX, windowPosY,
            windowWidth, windowHeight,
            0,
            CopyFromParent,
            InputOutput,
            CopyFromParent,
            valueMask,
            &attributes
        );

        if(!windowHandle)
            return false;

        XStoreName(windowDisplay, windowHandle, windowTitle.c_str());

        XSizeHints sizeHints{};
        sizeHints.flags = PMaxSize | PMinSize | USPosition;
        sizeHints.x = windowPosX;
        sizeHints.y = windowPosY;
        sizeHints.min_width = sizeHints.max_width = windowWidth;
        sizeHints.min_height = sizeHints.max_height = windowHeight;

        XWMHints wmHints{};
        wmHints.initial_state = NormalState;
        wmHints.input = true;
        wmHints.flags = StateHint | InputHint;
        XSetWMProperties(
            windowDisplay,
            windowHandle,
            nullptr,
            nullptr,
            nullptr,
            0,
            &sizeHints,
            &wmHints,
            nullptr
        );

        wmDeleteWindow = XInternAtom(windowDisplay, "WM_DELETE_WINDOW", false);
        wmProtocols = XInternAtom(windowDisplay, "WM_PROTOCOLS", false);
        if(!XSetWMProtocols(windowDisplay, windowHandle, &wmDeleteWindow, 1))
            return false;

        XMapRaised(windowDisplay, windowHandle);

        SetAtoms(windowDisplay, windowHandle, windowMode);

        XDefineCursor(windowDisplay, windowHandle, windowCursor);

        if(!windowIcon.empty())
            SetIcon(windowDisplay, windowHandle, windowIcon);

        XFlush(windowDisplay);

        return true;
    }

    void Window::WinProc(const XEvent * const event)
    {
        switch(event->type)
        {
        case FocusOut:
            if (lostFocus)
                lostFocus();
            break;

        case FocusIn:
            if (inFocus)
                inFocus();
            break;
        }
    }
}
#include "Window.h"
#include <unistd.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <png.h>

#define _NET_WM_STATE_REMOVE        0    // remove/unset property
#define _NET_WM_STATE_ADD           1    // add/set property
#define _NET_WM_STATE_TOGGLE        2    // toggle property

#define MWM_HINTS_DECORATIONS   2
#define MWM_DECOR_ALL           1

namespace Luna
{
    void (*Window::inFocus)() = nullptr;
    void (*Window::lostFocus)() = nullptr;

    Window::Window() noexcept 
        : window{}, 
        windowPosX{}, 
        windowPosY{}, 
        windowIcon{}
    {
        XInitThreads();

        display = XOpenDisplay(nullptr);
        screen = DefaultScreenOfDisplay(display);
        windowWidth = screen->width;
        windowHeight = screen->height;
        windowCursor = XCreateFontCursor(display, XC_left_ptr);
        windowColor.pixel = WhitePixel(display, DefaultScreen(display));
        windowTitle = string("Windows Game");
        windowMode = FULLSCREEN;
        windowCenterX = windowWidth / 2;
        windowCenterY = windowHeight / 2;
    }

    Window::~Window() noexcept
    {
        XFreeCursor(display, windowCursor);
        XUnmapWindow(display, window);
        XDestroyWindow(display, window);
        XCloseDisplay(display);
    }
    
    uint32 Window::GetColor(const char * color) noexcept
    {
        XColor hex{};
        XParseColor(display, DefaultColormap(display, 0), color, &hex);
        XAllocColor(display, DefaultColormap(display, 0), &hex);
        return hex.pixel;
    }

    void Window::Size(const uint32 width, const uint32 height) noexcept
    { 
        windowWidth = width; 
        windowHeight = height;

        windowCenterX = windowWidth / 2;
        windowCenterY = windowHeight / 2;

        windowPosX = (screen->width - windowWidth) / 2;
        windowPosY = (screen->height - windowHeight) / 2;
    }

    static void SendEventToWM(Display * display, XWindow window, Atom type, 
        long eventMask, long a, long b, long c = 0, long d = 0, long e = 0)
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
        Atom type, const unsigned char*	data, int nelements)
    {
        XChangeProperty(display, window,
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
        SendEventToWM(display, window, 
            wmProtocols, 
            NoEventMask, 
            wmDeleteWindow, 
            CurrentTime);
    }

    bool LoadPNG(const string_view filename, unsigned char ** imageData, int & width, int & height)
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

    void SetIcon(Display * display, XWindow window, const string_view filename)
    {
        int width, height;
        unsigned char * dataImage;
        LoadPNG(filename.data(), &dataImage, width, height);

        int longCount = 2 + width * height;

        unsigned long* icon = new unsigned long[longCount * sizeof(unsigned long)];
        unsigned long* target = icon;

        *target++ = width;
        *target++ = height;

        for (int i = 0; i < width * height; ++i)
        {
            *target++ = 
            ((dataImage[i * 4 + 2])) |
            ((dataImage[i * 4 + 1]) << 8) |
            ((dataImage[i * 4 + 0]) << 16) |
            ((dataImage[i * 4 + 3]) << 24);
        }

        Atom netWmIcon = XInternAtom(display, "_NET_WM_ICON", false);
        X11ChangeProperty(display, window,
            netWmIcon,
            XA_CARDINAL, 
            (unsigned char*) icon,
            longCount);

        delete[] icon;
    }

    void Fullscreen(Display *display, XWindow window)
    {
        Atom netWmState = XInternAtom(display, "_NET_WM_STATE", false);
        Atom netWmStateFullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", false);

        XSizeHints sizehints{};
        long flags{};
        XGetWMNormalHints(display, window, &sizehints, &flags);

        sizehints.flags &= ~(PMinSize | PMaxSize);
        XSetWMNormalHints(display, window, &sizehints);

        SendEventToWM(display, RootWindow(display, 0), 
            netWmState, 
            SubstructureNotifyMask | SubstructureRedirectMask, 
            _NET_WM_STATE_ADD, 
            netWmStateFullscreen);
    }

    void Borderless(Display * display, XWindow window)
    {
        struct
        {
            unsigned long flags;
            unsigned long functions;
            unsigned long decorations;
            long input_mode;
            unsigned long status;
        } hints = {};
    
        hints.flags = MWM_HINTS_DECORATIONS;
        
        Atom motifwmHints = XInternAtom(display, "_MOTIF_WM_HINTS", false);
        X11ChangeProperty(display, window,
            motifwmHints,
            motifwmHints, 
            (unsigned char*) &hints,
            sizeof(hints) / sizeof(long));
    }

    void SetAtoms(Display * display, XWindow window, uint32 windowMode)
    {
        if(windowMode == BORDERLESS)
            Borderless(display, window);
        else if(windowMode == FULLSCREEN)
            Fullscreen(display, window);

        auto pid = getpid();
        Atom newWmPid = XInternAtom(display, "_NET_WM_PID", false);
        X11ChangeProperty(display, window, 
            newWmPid, 
            XA_CARDINAL, 
            reinterpret_cast<unsigned char *>(&pid), 
            1
        );

        Atom newWmWindowType = XInternAtom(display, "_NET_WM_WINDOW_TYPE", false);
        Atom newWmWindowTypeNormal = XInternAtom(display, "_NET_WM_WINDOW_TYPE_NORMAL", false);
        X11ChangeProperty(display, window, 
            newWmWindowType,
            XA_ATOM, 
            reinterpret_cast<unsigned char *>(&newWmWindowTypeNormal), 
            1
        );

        long compositor = 1;
        Atom newWmBypassCompositor = XInternAtom(display, "_NET_WM_BYPASS_COMPOSITOR", false);
        X11ChangeProperty(display, window, 
            newWmBypassCompositor, 
            XA_CARDINAL, 
            reinterpret_cast<unsigned char *>(&compositor), 
            1
        );
    }

    bool Window::Create() noexcept
    {
        if(!display)
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
        
        window = XCreateWindow(display, RootWindow(display, 0),
            windowPosX, windowPosY,
            windowWidth, windowHeight,
            0, CopyFromParent, InputOutput, CopyFromParent,
            valueMask, &attributes
        );

        if(!window)
            return false;

        XStoreName(display, window, windowTitle.c_str());
        
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
        XSetWMProperties(display, window, nullptr, nullptr, nullptr, 0, &sizeHints, &wmHints, nullptr);

        wmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", false);
        wmProtocols = XInternAtom(display, "WM_PROTOCOLS", false);
        if(!XSetWMProtocols(display, window, &wmDeleteWindow, 1))
            return false;

        XMapRaised(display, window);

        SetAtoms(display, window, windowMode);

        XDefineCursor(display, window, windowCursor);
        
        if(!windowIcon.empty())
            SetIcon(display, window, windowIcon);
        
        XFlush(display);

        return true;
    }

    void Window::WinProc(XEvent * event)
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
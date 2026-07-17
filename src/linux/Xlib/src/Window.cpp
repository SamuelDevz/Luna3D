#include "Window.h"
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <png++/png.hpp>

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

    void Window::Close() noexcept
    {
        XEvent event{};
        event.type = ClientMessage;
        event.xclient.window = window;
        XSendEvent(display, window, false, NoEventMask, &event);
        XFlush(display);
    }

    void SetIcon(Display * display, ::Window window, const string_view filename)
    {    
        png::image<png::rgba_pixel> image(filename.data());
        int width = image.get_width();
        int height = image.get_height();

        unsigned long * iconData = new unsigned long[width * height];
        for (int y = 0; y < height; ++y) 
        {
            for (int x = 0; x < width; ++x) 
            {
                png::rgba_pixel pixel = image.get_pixel(x, y);
                unsigned long pixelValue = (pixel.alpha << 24) | (pixel.red << 16) | (pixel.green << 8) | pixel.blue;
                iconData[y * width + x] = pixelValue;
            }
        }

        unsigned long * data = new unsigned long[2 + width * height];
        data[0] = width;
        data[1] = height;
        for (int i = 0; i < width * height; ++i)
            data[i + 2] = iconData[i];

        XChangeProperty(display,
            window,
            XInternAtom(display, "_NET_WM_ICON", false),
            XA_CARDINAL,
            32,
            PropModeReplace,
            (unsigned char *)data,
            2 + width * height
        );

        delete[] iconData;
        delete[] data;
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
        XSetWMNormalHints(display, window, &sizeHints);

        XWMHints wmHints{};
        wmHints.initial_state = NormalState;
        wmHints.input = true;
        wmHints.flags = StateHint | IconPixmapHint | InputHint;

        XSetWMProperties(
            display, 
            window, 
            nullptr, 
            nullptr, 
            nullptr, 
            0, 
            &sizeHints, 
            &wmHints, 
            nullptr
        );
        
        wmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", false);
        wmProtocols = XInternAtom(display, "WM_PROTOCOLS", false);
        if(!XSetWMProtocols(display, window, &wmDeleteWindow, 1))
            return false;

        XMapRaised(display, window);

        XDefineCursor(display, window, windowCursor);
        
        if(!windowIcon.empty())
            SetIcon(display, window, windowIcon);
        
        XFlush(display);

        return true;
    }
}
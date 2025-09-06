#pragma once

#include "Types.h"
#include "Export.h"
#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/extensions/Xfixes.h>

namespace Luna
{
    enum WindowModes { FULLSCREEN, WINDOWED, BORDERLESS };

    class DLL Window
    {
    private:
        Display*    display;
        ::Window    window;
        Screen*     screen;
        int32		windowWidth;
        int32		windowHeight;
        string      windowIcon;
        ::Cursor	windowCursor;
        XColor	    windowColor;
        string		windowTitle;
        int32		windowMode;
        int32		windowPosX;
        int32		windowPosY;
        int32		windowCenterX;
        int32		windowCenterY;

        Atom wmDeleteWindow;
        Atom wmProtocols;

        static void (*inFocus)();
        static void (*lostFocus)();

        uint32 GetColor(const char * color) noexcept;
        
    public:
        explicit Window() noexcept;
        ~Window() noexcept;

        Display* XDisplay() const noexcept;
        ::Window XWindow() const noexcept;
        Atom WMDeleteWindow() const noexcept;
        int32 Width() const noexcept;
        int32 Height() const noexcept;
        int32 Mode() const noexcept;
        int32 CenterX() const noexcept;
        int32 CenterY() const noexcept;
        string Title() const noexcept;
        XColor Color() const noexcept;
        float AspectRatio() const noexcept;

        void Icon(const string_view filename) noexcept;
        void Cursor(const string_view filename) noexcept;
        void Title(const string_view title) noexcept;
        void Size(const uint32 width, const uint32 height) noexcept;
        void Mode(const uint32 mode) noexcept;
        void Color(const string_view hex) noexcept;

        void HideCursor(const bool hide) const noexcept;
        void Close() noexcept;
        bool Create() noexcept;

        void InFocus(void(*func)()) noexcept;
        void LostFocus(void(*func)()) noexcept;
    };

    inline Display* Window::XDisplay() const noexcept
    { return display; }

    inline ::Window Window::XWindow() const noexcept
    { return window; }
        
    inline Atom Window::WMDeleteWindow() const noexcept
    { return wmDeleteWindow; }

    inline int32 Window::Width() const noexcept
    { return windowWidth; }

    inline int32 Window::Height() const noexcept
    { return windowHeight; }

    inline int32 Window::Mode() const noexcept
    { return windowMode; }

    inline int32 Window::CenterX() const noexcept
    { return windowCenterX; }

    inline int32 Window::CenterY() const noexcept
    { return windowCenterY; }

    inline string Window::Title() const noexcept
    { return windowTitle; }

    inline XColor Window::Color() const noexcept
    { return windowColor; }

    inline float Window::AspectRatio() const noexcept
    { return windowWidth / float(windowHeight); }

    inline void Window::Icon(const string_view filename) noexcept
    { windowIcon = filename; }

    inline void Window::Cursor(const string_view filename) noexcept
    { windowCursor = XcursorFilenameLoadCursor(display, filename.data()); }

    inline void Window::Title(const string_view title) noexcept
    { windowTitle = title; }

    inline void Window::Mode(const uint32 mode) noexcept
    { windowMode = mode; }

    inline void Window::Color(const string_view hex) noexcept
    { windowColor.pixel = GetColor(hex.data()); }
    
    inline void Window::HideCursor(const bool hide) const noexcept
    { hide ? XFixesHideCursor(display, window) : XFixesShowCursor(display, window); }

    inline void Window::InFocus(void(*func)()) noexcept
    { inFocus = func; }

    inline void Window::LostFocus(void(*func)()) noexcept
    { lostFocus = func; }
}
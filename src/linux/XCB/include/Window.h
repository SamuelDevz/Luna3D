#pragma once

#include "Types.h"
#include "Export.h"
#include <xcb/xcb.h>
#include <xcb/xfixes.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xcursor/Xcursor.h>

namespace Luna
{
    enum WindowModes { FULLSCREEN, WINDOWED, BORDERLESS };

    class DLL Window
    {
    private:
        Display*          display;
        xcb_connection_t* connection;
        xcb_window_t      window;
        xcb_screen_t*     screen;
        int32		      windowWidth;
        int32		      windowHeight;
        string            windowIcon;
        xcb_cursor_t	  windowCursor;
        xcb_colormap_t    windowColor;
        string		      windowTitle;
        int32		      windowMode;
        int32		      windowPosX;
        int32		      windowPosY;
        int32		      windowCenterX;
        int32		      windowCenterY;

        xcb_atom_t wmDeleteWindow;
        xcb_atom_t wmProtocols;

        static void (*inFocus)();
        static void (*lostFocus)();

        uint32 GetColor(const string hexColor) noexcept;
        
    public:
        explicit Window() noexcept;
        ~Window() noexcept;

        xcb_connection_t* Connection() const noexcept;
        xcb_window_t Id() const noexcept;
        xcb_atom_t WMDeleteWindow() const noexcept;
        int32 Width() const noexcept;
        int32 Height() const noexcept;
        int32 Mode() const noexcept;
        int32 CenterX() const noexcept;
        int32 CenterY() const noexcept;
        string Title() const noexcept;
        xcb_colormap_t Color() const noexcept;
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

    inline xcb_connection_t* Window::Connection() const noexcept
    { return connection; }

    inline xcb_window_t Window::Id() const noexcept
    { return window; }
        
    inline xcb_atom_t Window::WMDeleteWindow() const noexcept
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

    inline xcb_colormap_t Window::Color() const noexcept
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
    { windowColor = GetColor(hex.data()); }
    
    inline void Window::HideCursor(const bool hide) const noexcept
    { hide ? xcb_xfixes_hide_cursor(connection, window) : xcb_xfixes_show_cursor(connection, window); }

    inline void Window::InFocus(void(*func)()) noexcept
    { inFocus = func; }

    inline void Window::LostFocus(void(*func)()) noexcept
    { lostFocus = func; }
}
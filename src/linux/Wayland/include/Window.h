#pragma once

#include "Types.h"
#include "Export.h"
#include <wayland-client.h>
#include <wayland-cursor.h>
#include "xdg-shell-client-protocol.h"

namespace Luna
{
    enum WindowModes { FULLSCREEN, WINDOWED, BORDERLESS };

    class DLL Window
    {
    private:
    
        wl_display*           display;
        wl_registry *         registry;
        wl_surface *          window;
        xdg_surface *         xdgSurface;
        xdg_toplevel *        xdgToplevel;

        int32		          windowWidth;
        int32		          windowHeight;
        // string          windowIcon;
        wl_cursor	    windowCursor;
        // XColor	        windowColor;
        string		    windowTitle;
        int32		    windowMode;
        int32		    windowPosX;
        int32		    windowPosY;
        int32		    windowCenterX;
        int32		    windowCenterY;

        static void (*inFocus)();
        static void (*lostFocus)();

        static wl_compositor *compositor;
        static xdg_wm_base * wm_base;
        static wl_shm * shm;
        static wl_buffer * buffer;

        static xdg_toplevel_listener * toplevel_listener;

        static void registry_handle_global(
            void *data, 
            struct wl_registry *registry,
            uint32_t name, 
            const char *interface, 
            uint32_t version);

    public:
        explicit Window() noexcept;
        ~Window() noexcept;

        wl_display* Display() const noexcept;
        wl_surface* Surface() const noexcept;
        xdg_surface* XDGSurface() const noexcept;
        xdg_toplevel* XDGTopLevel() const noexcept;
        xdg_wm_base* WMBase() const noexcept;

        int32 Width() const noexcept;
        int32 Height() const noexcept;
        int32 Mode() const noexcept;
        int32 CenterX() const noexcept;
        int32 CenterY() const noexcept;
        string Title() const noexcept;
        // XColor Color() const noexcept;
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

        static xdg_toplevel_listener* XDGTopLevelListener() noexcept;
    };

    inline wl_display* Window::Display() const noexcept
    { return display; }

    inline wl_surface* Window::Surface() const noexcept
    { return window; }

    inline xdg_surface* Window::XDGSurface() const noexcept
    { return xdgSurface; }

    inline xdg_toplevel* Window::XDGTopLevel() const noexcept
    { return xdgToplevel; }

    inline xdg_wm_base* Window::WMBase() const noexcept
    { return wm_base; }

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

    // inline XColor Window::Color() const noexcept
    // { return windowColor; }

    inline float Window::AspectRatio() const noexcept
    { return windowWidth / float(windowHeight); }

    // inline void Window::Icon(const string_view filename) noexcept
    // { windowIcon = filename; }

    // inline void Window::Cursor(const string_view filename) noexcept
    // { windowCursor = XcursorFilenameLoadCursor(display, filename.data()); }

    inline void Window::Title(const string_view title) noexcept
    { windowTitle = title; }

    inline void Window::Mode(const uint32 mode) noexcept
    { windowMode = mode; }

    // inline void Window::Color(const string_view hex) noexcept
    // { windowColor.pixel = GetColor(hex.data()); }
    
    // inline void Window::HideCursor(const bool hide) const noexcept
    // { hide ? XFixesHideCursor(display, window) : XFixesShowCursor(display, window); }

    inline void Window::InFocus(void(*func)()) noexcept
    { inFocus = func; }

    inline void Window::LostFocus(void(*func)()) noexcept
    { lostFocus = func; }

    inline xdg_toplevel_listener* Window::XDGTopLevelListener() noexcept
    { return toplevel_listener; }
}
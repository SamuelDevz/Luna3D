#pragma once

#include "Types.h"
#include "Export.h"
#include <wayland-client.h>
#include <wayland-cursor.h>
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-unstable-v1.h"

namespace Luna
{
    enum WindowModes { FULLSCREEN, WINDOWED, BORDERLESS };

    struct Point
    {
        int32 x, y;
    };

    struct Resolution
    {
        uint32 width, height;
    };

    struct OutputInfo
    {
        string deviceName;
        union {
            Point position;
            struct {
                Point position;
                uint32 displayOrientation;
            } DUMMYSTRUCTNAME2;
        } DUMMYUNIONNAME;
        Resolution resolution;
        Resolution physicalSize;
        uint32 refreshRate;
        int32 scale;
        uint32 dpi;
        uint32 mode;
    };

    class DLL Window
    {
    private:
        wl_display*                         display;
        wl_registry *                       registry;
        wl_surface *                        window;
        xdg_surface *                       xdgSurface;
        xdg_toplevel *                      xdgToplevel;
        zxdg_toplevel_decoration_v1*        decoration;
        int32		                        windowWidth;
        int32		                        windowHeight;
        uint32	                            windowColor;
        string		                        windowTitle;
        int32		                        windowMode;
        int32		                        windowPosX;
        int32		                        windowPosY;
        int32		                        windowCenterX;
        int32		                        windowCenterY;

        static void (*inFocus)();
        static void (*lostFocus)();

        static wl_compositor *              compositor;
        static xdg_wm_base *                wmBase;
        static wl_shm *                     shm;
        static wl_buffer *                  buffer;
        static xdg_toplevel_listener *      toplevelListener;
        static zxdg_decoration_manager_v1*  decoManager;
        static wl_output *                  output;
        static OutputInfo *                 monitor;

        static void OutputHandleGeometry(void *userData, wl_output *wl_output,
            int32 x, int32 y,
            int32 physical_width, int32 physical_height, 
            int32 subpixel, const char *make,
            const char *model, int32 transform);

        static void OutputHandleMode(void *userData, wl_output *wl_output,
            uint32 flags, 
            int32 width, int32 height,
            int32 refresh);

        static void OutputHandleScale(void *userData, wl_output *wl_output, 
            int32 factor);

        static void OutputHandleName(void *userData, wl_output *wl_output, 
            const char *name);

        static void OutputHandleDescription(void *userData, wl_output *wl_output, 
            const char *description);
            
        static void OutputHandleDone(void *userData, wl_output *wl_output);

        static void RegistryHandleGlobal(
            void *userData,
            struct wl_registry *registry,
            uint32_t name,
            const char *interface,
            uint32_t version);

        uint32 GetColor(const string hexColor) noexcept;

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
        uint32 Color() const noexcept;
        float AspectRatio() const noexcept;

        void Icon(const string_view filename) noexcept;
        void Cursor(const string_view filename) noexcept;
        void Title(const string_view title) noexcept;
        void Size(const uint32 width, const uint32 height) noexcept;
        void Mode(const uint32 mode) noexcept;
        void Color(const string_view hex) noexcept;

        // void HideCursor(const bool hide) const noexcept;
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
    { return wmBase; }

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

    inline uint32 Window::Color() const noexcept
    { return windowColor; }

    inline float Window::AspectRatio() const noexcept
    { return windowWidth / float(windowHeight); }

    inline void Window::Icon(const string_view filename) noexcept
    {}

    inline void Window::Cursor(const string_view filename) noexcept
    {}

    inline void Window::Title(const string_view title) noexcept
    { windowTitle = title; }

    inline void Window::Mode(const uint32 mode) noexcept
    { windowMode = mode; }

    inline void Window::Color(const string_view hex) noexcept
    { windowColor = GetColor(hex.data()); }

    // inline void Window::HideCursor(const bool hide) const noexcept
    // {}

    inline void Window::InFocus(void(*func)()) noexcept
    { inFocus = func; }

    inline void Window::LostFocus(void(*func)()) noexcept
    { lostFocus = func; }

    inline xdg_toplevel_listener* Window::XDGTopLevelListener() noexcept
    { return toplevelListener; }
}
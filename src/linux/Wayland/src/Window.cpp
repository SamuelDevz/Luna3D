#include "Window.h"
#include <cstdio>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <wayland-client-protocol.h>

namespace Luna
{
    wl_compositor * Window::compositor = nullptr;
    xdg_wm_base * Window::wm_base = nullptr;
    wl_shm * Window::shm = nullptr;
    wl_buffer * Window::buffer = nullptr;

    xdg_toplevel_listener* Window::toplevel_listener = nullptr;

    void (*Window::inFocus)() = nullptr;
    void (*Window::lostFocus)() = nullptr;

    Window::Window() noexcept : windowPosX{}, windowPosY{}
    {
        display = wl_display_connect(nullptr);
        
        windowWidth = 1920;
        windowHeight = 1080;
        // windowCursor = XCreateFontCursor(display, XC_left_ptr);
        windowTitle = string("Windows Game");
        windowMode = FULLSCREEN;
        windowCenterX = windowWidth / 2;
        windowCenterY = windowHeight / 2;
    }

    Window::~Window() noexcept
    {
        if (buffer)
            wl_buffer_destroy(buffer);

        xdg_toplevel_destroy(xdgToplevel);
        xdg_surface_destroy(xdgSurface);
        wl_surface_destroy(window);
        xdg_wm_base_destroy(wm_base);
        wl_compositor_destroy(compositor);
        wl_shm_destroy(shm);
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
        
        delete toplevel_listener;
    }

    void Window::Size(const uint32 width, const uint32 height) noexcept
    { 
        windowWidth = width; 
        windowHeight = height;

        windowCenterX = windowWidth / 2;
        windowCenterY = windowHeight / 2;

        windowPosX = (1920 - windowWidth) / 2;
        windowPosY = (1080 - windowHeight) / 2;
    }

    void Window::Close() noexcept
    {
    }

    static void xdg_wm_base_ping(void *data, xdg_wm_base *wm_base, uint32_t serial)
    {
        xdg_wm_base_pong(wm_base, serial);
    }

    void Window::registry_handle_global(void *data, wl_registry *registry,
        uint32_t id, const char *interface,
        uint32_t version)
    {
        static const xdg_wm_base_listener xdg_wm_base_listener = {
            .ping = xdg_wm_base_ping,
        };

        const string _interface(interface);
        if (_interface == wl_compositor_interface.name)
        {
            compositor = reinterpret_cast<wl_compositor *>(wl_registry_bind(registry, id, &wl_compositor_interface, 4));
        }
        else if (_interface == xdg_wm_base_interface.name)
        {
            wm_base = reinterpret_cast<xdg_wm_base *>(wl_registry_bind(registry, id, &xdg_wm_base_interface, 1));
            xdg_wm_base_add_listener(wm_base, &xdg_wm_base_listener, nullptr);
        }
        else if (_interface == wl_shm_interface.name)
        {
            shm = reinterpret_cast<wl_shm *>(wl_registry_bind(registry, id, &wl_shm_interface, 1));
        }
    }

    static void registry_global_remove(void *data, wl_registry *registry, uint32_t id) {}

    static void toplevel_configure(void *data, struct xdg_toplevel *toplevel,
        int32_t width, int32_t height,
        struct wl_array *states)
    {
    }

    wl_buffer* CreateShmBuffer(int32 width, int32 height, uint32 color, wl_shm * shm)
    {
        int stride = width * 4;
        int size = stride * height; // bytes

        // open an anonymous file and write some zero bytes to it
        int fd = syscall(SYS_memfd_create, "buffer", 0);
        ftruncate(fd, size);

        // map it to the memory
        void * data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        // change color the buffer 
        uint32_t *pixel = reinterpret_cast<uint32_t*>(data);
        for (uint32 i = 0; i < size / 4; ++i)
            pixel[i] = color;

        wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
        wl_buffer *buffer = wl_shm_pool_create_buffer(
            pool, 
            0, 
            width, 
            height, 
            stride, 
            WL_SHM_FORMAT_XRGB8888
        );

        wl_shm_pool_destroy(pool);
        close(fd);

        return buffer;
    }

    bool Window::Create() noexcept
    {
        if(!display)
            return false;

        const wl_registry_listener registry_listener = {
            .global = registry_handle_global,
            .global_remove = registry_global_remove,
        };

        registry = wl_display_get_registry(display);
        wl_registry_add_listener(registry, &registry_listener, nullptr);
        wl_display_roundtrip(display);

        window = wl_compositor_create_surface(compositor);
        xdgSurface = xdg_wm_base_get_xdg_surface(wm_base, window);
        xdgToplevel = xdg_surface_get_toplevel(xdgSurface);

        toplevel_listener = new xdg_toplevel_listener{
            .configure = toplevel_configure,
            // .close = nullptr
        };
        
        xdg_toplevel_add_listener(xdgToplevel, toplevel_listener, nullptr);
        xdg_toplevel_set_title(xdgToplevel, windowTitle.c_str());

        wl_surface_commit(window);
        wl_display_roundtrip(display);

        constexpr const uint32 color = 0xff007acc;
        wl_buffer * buffer = CreateShmBuffer(windowWidth, windowHeight, color, shm);

        wl_surface_commit(window);
        wl_surface_attach(window, buffer, 0, 0);
        wl_surface_commit(window);

        return true;
    }
}
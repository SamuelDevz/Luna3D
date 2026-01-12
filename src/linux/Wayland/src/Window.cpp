#include "Window.h"
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
    zxdg_decoration_manager_v1* Window::deco_manager = nullptr;
    wl_output * Window::output = nullptr;

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

        wl_output_destroy(output);        
        zxdg_toplevel_decoration_v1_destroy(decoration);
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

    /******************************/
    /******XDG Window Manager******/
    /******************************/

    static void xdg_wm_base_ping(void *data, xdg_wm_base *wm_base, uint32_t serial)
    {
        xdg_wm_base_pong(wm_base, serial);
    }

    /******************************/
    /*********XDG Surface**********/
    /******************************/

    static void surface_configure(void *data, xdg_surface *xdg_surface, uint32_t serial)
    {
        xdg_surface_ack_configure(xdg_surface, serial);
    }

    /******************************/
    /********XDG Toplevel**********/
    /******************************/

    static void toplevel_configure(void *data, xdg_toplevel *toplevel,
        int32_t width, int32_t height, wl_array *states)
    {
    }

    static void toplevel_configure_bounds(void *data, xdg_toplevel *xdg_toplevel,
        int32_t width, int32_t height)
    {
    }

    static void toplevel_wm_capabilities(void *data, xdg_toplevel *xdg_toplevel,
        wl_array *capabilities)
    {
    }

    /******************************/
    /***********Registry***********/
    /******************************/

    void Window::registry_handle_global(void *data, wl_registry *registry,
        uint32_t id, const char *interface, uint32_t version)
    {
        static const xdg_wm_base_listener xdg_wm_base_listener = {
            .ping = xdg_wm_base_ping,
        };

        const string _interface(interface);
        if (_interface == wl_compositor_interface.name)
        {
            compositor = reinterpret_cast<wl_compositor *>(wl_registry_bind(registry, id, &wl_compositor_interface, 4));
        }
        else if (_interface == wl_output_interface.name) 
        {
            output = reinterpret_cast<wl_output*>(wl_registry_bind(registry, id, &wl_output_interface, version));
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
        else if (_interface == zxdg_decoration_manager_v1_interface.name)
        {
            deco_manager = reinterpret_cast<zxdg_decoration_manager_v1*>(wl_registry_bind(registry, id, &zxdg_decoration_manager_v1_interface, 1));
        }
    }

    static void registry_global_remove(void *data, wl_registry *registry, uint32_t id) {}

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

    void output_geometry(void* data, wl_output *wl_output,
        int x, int y, int physical_width,
        int physical_height, int subpixel, const char *make,
        const char *model, int transform) 
    {
        printf("=== Monitor Geometry Information ===\n");
        printf("Position: (%d, %d)\n", x, y);
        printf("Physical dimensions: %d x %d mm\n", physical_width, physical_height);
        
        const char* subpixel_str = "unknown";
        switch(subpixel) 
        {
            case WL_OUTPUT_SUBPIXEL_NONE: subpixel_str = "none"; break;
            case WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB: subpixel_str = "horizontal RGB"; break;
            case WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR: subpixel_str = "horizontal BGR"; break;
            case WL_OUTPUT_SUBPIXEL_VERTICAL_RGB: subpixel_str = "vertical RGB"; break;
            case WL_OUTPUT_SUBPIXEL_VERTICAL_BGR: subpixel_str = "vertical BGR"; break;
        }
        printf("Subpixel: %s\n", subpixel_str);
        
        if (make) printf("Manufacturer: %s\n", make);
        if (model) printf("Model: %s\n", model);
        
        const char* transform_str = "normal";
        switch(transform) 
        {
            case WL_OUTPUT_TRANSFORM_90: transform_str = "90°"; break;
            case WL_OUTPUT_TRANSFORM_180: transform_str = "180°"; break;
            case WL_OUTPUT_TRANSFORM_270: transform_str = "270°"; break;
            case WL_OUTPUT_TRANSFORM_FLIPPED: transform_str = "flipped"; break;
            case WL_OUTPUT_TRANSFORM_FLIPPED_90: transform_str = "flipped 90°"; break;
            case WL_OUTPUT_TRANSFORM_FLIPPED_180: transform_str = "flipped 180°"; break;
            case WL_OUTPUT_TRANSFORM_FLIPPED_270: transform_str = "flipped 270°"; break;
        }
        printf("Transform: %s\n", transform_str);
        printf("\n");
    }

    void output_mode(void *data, wl_output *wl_output,
        uint flags, int width, int height, int refresh) 
    {
        printf("=== Monitor Video Mode ===\n");
        printf("Resolution: %d x %d pixels\n", width, height);
        printf("Refresh rate: %.2f Hz\n", refresh / 1000.0f);
        
        bool is_current = (flags & WL_OUTPUT_MODE_CURRENT) != 0;
        bool is_preferred = (flags & WL_OUTPUT_MODE_PREFERRED) != 0;
        
        printf("Flags: ");
        if (is_current) printf("[CURRENT] ");
        if (is_preferred) printf("[PREFERRED] ");
        if (!is_current && !is_preferred) printf("none");
        printf("\n\n");
    }
    
    void output_scale(void *data, wl_output *wl_output, int factor) 
    {
        printf("=== Monitor Scale ===\n");
        printf("Scale factor: %d\n\n", factor);
    }
    
    void output_name(void *data, wl_output *wl_output, const char *name) 
    {
        printf("=== Monitor Name ===\n");
        if (name) printf("Name: %s\n\n", name);
        else      printf("Name: not available\n\n");
    }
    
    void output_description(void *data, wl_output *wl_output, const char *description) 
    {
        printf("=== Monitor Description ===\n");
        if (description) printf("Description: %s\n\n", description);
        else             printf("Description: not available\n\n");
    }

    void output_done(void *data, wl_output *wl_output) 
    {
        printf("=== Monitor Configuration Complete ===\n");
        printf("All monitor information has been received.\n\n");
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

        const wl_output_listener output_listener = {
            .geometry = output_geometry,
            .mode = output_mode,
            .done = output_done,
            .scale = output_scale,
            .name = output_name,
            .description = output_description,
        };

        wl_output_add_listener(output, &output_listener, nullptr);
        window = wl_compositor_create_surface(compositor);
        // wl_surface_add_listener(window, &surface_listener, window);
        xdgSurface = xdg_wm_base_get_xdg_surface(wm_base, window);

        static const xdg_surface_listener xdg_surface_listener = {
            .configure = surface_configure
        };
        xdg_surface_add_listener(xdgSurface, &xdg_surface_listener, window);
        wl_display_roundtrip(display);

        xdgToplevel = xdg_surface_get_toplevel(xdgSurface);

        toplevel_listener = new xdg_toplevel_listener {
            .configure = toplevel_configure,
            .configure_bounds = toplevel_configure_bounds,
            .wm_capabilities = toplevel_wm_capabilities
        };

        xdg_toplevel_add_listener(xdgToplevel, toplevel_listener, nullptr);
        xdg_toplevel_set_title(xdgToplevel, windowTitle.c_str());
        xdg_toplevel_set_app_id(xdgToplevel, windowTitle.c_str());

        if(windowMode == FULLSCREEN)
            xdg_toplevel_set_fullscreen(xdgToplevel, nullptr);

        if(windowMode == WINDOWED)
        {
            if (deco_manager)
            {
                decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(deco_manager, xdgToplevel);

                zxdg_toplevel_decoration_v1_set_mode(
                    decoration,
                    ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE
                );
            }
        }

        wl_surface_commit(window);
        wl_display_roundtrip(display);

        constexpr const uint32 color = 0xff007acc;
        wl_buffer * buffer = CreateShmBuffer(windowWidth, windowHeight, color, shm);

        wl_surface_attach(window, buffer, 0, 0);
        wl_surface_commit(window);

        return true;
    }
}
#include "Window.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>

namespace Luna
{
    wl_compositor * Window::compositor = nullptr;
    xdg_wm_base * Window::wmBase = nullptr;
    wl_shm * Window::shm = nullptr;
    wl_buffer * Window::buffer = nullptr;
    xdg_toplevel_listener* Window::toplevelListener = nullptr;
    zxdg_decoration_manager_v1* Window::decoManager = nullptr;
    wl_output * Window::output = nullptr;
    OutputInfo * Window::monitor = nullptr;

    void (*Window::inFocus)() = nullptr;
    void (*Window::lostFocus)() = nullptr;

    Window::Window() noexcept
        : registry{nullptr},
        window{nullptr},
        xdgSurface{nullptr},
        xdgToplevel{nullptr},
        decoration{nullptr},
        windowWidth{},
        windowHeight{},
        windowPosX{},
        windowPosY{}
    {
        display = wl_display_connect(nullptr);  
        windowColor = 0xFFFFFF;
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
        xdg_wm_base_destroy(wmBase);
        wl_compositor_destroy(compositor);
        wl_shm_destroy(shm);
        wl_registry_destroy(registry);
        wl_display_disconnect(display);

        delete monitor;
        delete toplevelListener;
    }

    uint32 Window::GetColor(const string hexColor) noexcept
    {
        const string redStr = hexColor.substr(1, 2);
        const string greenStr = hexColor.substr(3, 2);
        const string blueStr = hexColor.substr(5, 2);

        auto redLong = strtol(redStr.c_str(), nullptr, 16);
        auto greenLong = strtol(greenStr.c_str(), nullptr, 16);
        auto blueLong = strtol(blueStr.c_str(), nullptr, 16);

        uint32 color = (redLong << 16) | (greenLong << 8) | blueLong;

        return color;
    }

    void Window::Size(const uint32 width, const uint32 height) noexcept
    {
        windowWidth = width;
        windowHeight = height;

        windowCenterX = windowWidth / 2;
        windowCenterY = windowHeight / 2;
    }

    void Window::Close() noexcept
    {
    }

    /******************************/
    /******XDG Window Manager******/
    /******************************/

    static void XdgWmBasePing(void *userData, xdg_wm_base *base, uint32 serial)
    {
        xdg_wm_base_pong(base, serial);
    }

    /******************************/
    /*********XDG Surface**********/
    /******************************/

    static void SurfaceConfigure(void *userData, xdg_surface *surface, uint32 serial)
    {
        xdg_surface_ack_configure(surface, serial);
    }    

    /******************************/
    /************Output************/
    /******************************/

    void Window::OutputHandleGeometry(void* userData, wl_output *wl_output,
        int32 x, int32 y, 
        int32 physicalWidth, int32 physicalHeight, 
        int32 subpixel, const char *make,
        const char *model, int32 transform) 
    {
        monitor->physicalSize.width = physicalWidth;
        monitor->physicalSize.height = physicalHeight;
        monitor->DUMMYUNIONNAME.position.x = x;
        monitor->DUMMYUNIONNAME.position.y = y;
        // monitor->subpixel = subpixel;
        // monitor->DUMMYUNIONNAME.DUMMYSTRUCTNAME2.displayOrientation = transform;
    }

    static int32 Euclid(int32 a, int32 b)
    {
        return b ? Euclid(b, a % b) : a;
    }

    void Window::OutputHandleMode(void *userData, wl_output *wl_output,
        uint32 flags, int32 width, int32 height, int32 refresh) 
    {
        if (flags & WL_OUTPUT_MODE_CURRENT) 
        {
            monitor->resolution.width = width;
            monitor->resolution.height = height;
            monitor->refreshRate = refresh / 1000;
        }
        monitor->mode = flags;
    }

    void Window::OutputHandleScale(void *userData, wl_output *wl_output, int32 factor) 
    {
        monitor->scale = factor;
    }
    
    void Window::OutputHandleName(void *userData, wl_output *wl_output, const char *name) 
    {
        monitor->deviceName = name ? string(name) : "";
    }

    void Window::OutputHandleDone(void *userData, wl_output *wl_output) 
    {
        if (!monitor)
            return;

        if (monitor->physicalSize.width <= 0 || monitor->physicalSize.height <= 0)
        {
            // If Wayland does not provide a physical size, assume the default 96 DPI
            monitor->dpi = 96;
            monitor->physicalSize.width  = static_cast<uint32>(monitor->resolution.width * 25.4f / monitor->dpi);
            monitor->physicalSize.height = static_cast<uint32>(monitor->resolution.height * 25.4f / monitor->dpi);
        }
        else
        {
            monitor->dpi = static_cast<uint32>(monitor->resolution.width * 25.4f / monitor->physicalSize.width);
        }

        if (!monitor->deviceName.empty()) 
            printf("Name: %s\n", monitor->deviceName.c_str());
        else
            printf("Name: not available\n");

        switch (monitor->mode) 
        {
            case WL_OUTPUT_MODE_CURRENT:
                printf("Current mode: ");
                break;
            
            case WL_OUTPUT_MODE_PREFERRED:
                printf("Preferred mode: ");
                break;
            
            default: printf("None mode: ");
        }

        const int32 gcd = Euclid(monitor->resolution.width, monitor->resolution.height);

        printf("%d x %d (%d:%d) %dHz\n", 
            monitor->resolution.width, 
            monitor->resolution.height, 
            monitor->resolution.width / gcd, 
            monitor->resolution.height / gcd,
            monitor->refreshRate
        );
        printf("Virtual position: (%d, %d)\n", monitor->DUMMYUNIONNAME.position.x, monitor->DUMMYUNIONNAME.position.y);
        printf("Scale factor: %d\n", monitor->scale);
        printf("Physical size: %d x %d mm (%d dpi at %d x %d)\n", 
            monitor->physicalSize.width, 
            monitor->physicalSize.height,
            monitor->dpi,
            monitor->resolution.width,
            monitor->resolution.height
        );

        /*
        // Print subpixel information
        const char* subpixel_str = "unknown";
        switch(monitor->subpixel) 
        {
            case WL_OUTPUT_SUBPIXEL_NONE: subpixel_str = "none"; break;
            case WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB: subpixel_str = "horizontal RGB"; break;
            case WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR: subpixel_str = "horizontal BGR"; break;
            case WL_OUTPUT_SUBPIXEL_VERTICAL_RGB: subpixel_str = "vertical RGB"; break;
            case WL_OUTPUT_SUBPIXEL_VERTICAL_BGR: subpixel_str = "vertical BGR"; break;
        }
        printf("Subpixel: %s\n", subpixel_str);
        
        const char* transform_str = "normal";
        switch(monitor->DUMMYUNIONNAME.DUMMYSTRUCTNAME2.displayOrientation) 
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
        */
    }

    /******************************/
    /***********Registry***********/
    /******************************/

    static void RegistryGlobalRemove(void *userData, wl_registry *registry, uint32 id) {}

    void Window::RegistryHandleGlobal(void *userData, wl_registry *registry,
        uint32 id, const char *interface, uint32 version)
    {
        static const xdg_wm_base_listener wmBaseListener = {
            .ping = XdgWmBasePing,
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
            wmBase = reinterpret_cast<xdg_wm_base *>(wl_registry_bind(registry, id, &xdg_wm_base_interface, 1));
            xdg_wm_base_add_listener(wmBase, &wmBaseListener, nullptr);
        }
        else if (_interface == wl_shm_interface.name)
        {
            shm = reinterpret_cast<wl_shm *>(wl_registry_bind(registry, id, &wl_shm_interface, 1));
        }
        else if (_interface == zxdg_decoration_manager_v1_interface.name)
        {
            decoManager = reinterpret_cast<zxdg_decoration_manager_v1*>(wl_registry_bind(registry, id, &zxdg_decoration_manager_v1_interface, 1));
        }
    }

    wl_buffer* CreateShmBuffer(int32 width, int32 height, uint32 color, wl_shm * shm)
    {
        int32 stride = width * 4;
        int32 size = stride * height; // bytes

        // open an anonymous file and write some zero bytes to it
        int32 fd = syscall(SYS_memfd_create, "buffer", 0);
        ftruncate(fd, size);

        // map it to the memory
        void * data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        // change color the buffer
        uint32 *pixel = reinterpret_cast<uint32*>(data);
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

        const wl_registry_listener registryListener = {
            .global = RegistryHandleGlobal,
            .global_remove = RegistryGlobalRemove,
        };

        registry = wl_display_get_registry(display);
        wl_registry_add_listener(registry, &registryListener, nullptr);
        wl_display_roundtrip(display);

        monitor = new OutputInfo;

        const wl_output_listener outputListener = {
            .geometry = OutputHandleGeometry,
            .mode = OutputHandleMode,
            .done = OutputHandleDone,
            .scale = OutputHandleScale,
            .name = OutputHandleName,
            .description = [](void*, wl_output*, const char*) {},
        };

        wl_output_add_listener(output, &outputListener, nullptr);
        wl_display_roundtrip(display);
        windowPosX = (monitor->resolution.width - windowWidth) / 2;
        windowPosY = (monitor->resolution.height - windowHeight) / 2;

        if((windowWidth == 0 || windowHeight == 0) && windowMode == FULLSCREEN)
        {
            windowWidth = monitor->resolution.width;
            windowHeight = monitor->resolution.height;
        }

        window = wl_compositor_create_surface(compositor);
        xdgSurface = xdg_wm_base_get_xdg_surface(wmBase, window);

        static const xdg_surface_listener xdgSurfaceListener = {
            .configure = SurfaceConfigure
        };
        xdg_surface_add_listener(xdgSurface, &xdgSurfaceListener, window);
        wl_display_roundtrip(display);

        xdgToplevel = xdg_surface_get_toplevel(xdgSurface);

        toplevelListener = new xdg_toplevel_listener {
            .configure = [](void*, xdg_toplevel*, int32, int32, wl_array*) {},
            .configure_bounds = [](void*, xdg_toplevel*, int32, int32) {},
            .wm_capabilities = [](void*, xdg_toplevel*, wl_array*) {}
        };

        xdg_toplevel_add_listener(xdgToplevel, toplevelListener, nullptr);
        xdg_toplevel_set_title(xdgToplevel, windowTitle.c_str());
        xdg_toplevel_set_app_id(xdgToplevel, windowTitle.c_str());

        if(windowMode == FULLSCREEN)
            xdg_toplevel_set_fullscreen(xdgToplevel, nullptr);

        if(windowMode != BORDERLESS)
        {
            if (decoManager)
            {
                decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(decoManager, xdgToplevel);

                zxdg_toplevel_decoration_v1_set_mode(
                    decoration,
                    ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE
                );
            }
        }

        wl_surface_commit(window);
        wl_display_roundtrip(display);

        wl_buffer * buffer = CreateShmBuffer(windowWidth, windowHeight, windowColor, shm);
        wl_surface_attach(window, buffer, 0, 0);
        wl_surface_commit(window);

        return true;
    }
}
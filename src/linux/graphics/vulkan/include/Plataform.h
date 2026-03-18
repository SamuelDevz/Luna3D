#pragma once

#ifdef PLATFORM_XLIB
    #include <vulkan/vulkan_xlib.h>
#elif PLATAFORM_XCB
    #include <vulkan/vulkan_xcb.h>
#elif PLATAFORM_WAYLAND
    #include <vulkan/vulkan_wayland.h>
#endif
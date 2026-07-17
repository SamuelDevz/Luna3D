#include "Graphics.h"
#include "VkError.h"
#include "Utils.h"

#ifdef PLATAFORM_XLIB
    #include <vulkan/vulkan_xlib.h>
#endif

namespace Luna
{
    Graphics::Graphics() noexcept : instance{nullptr}
    {
    }

    Graphics::~Graphics() noexcept
    {
        vkDestroyInstance(instance, nullptr);
    }

    void Graphics::Initialize(const Window * const window)
    {
        // ---------------------------------------------------
        // Instance
        // ---------------------------------------------------

        constexpr const char* instanceLayers[]
        {
            "VK_LAYER_KHRONOS_validation",
        };

        constexpr const char* instanceExtensions[]
        {
            VK_KHR_SURFACE_EXTENSION_NAME,
        #ifdef PLATAFORM_XLIB
            VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
        #endif
        #ifdef _DEBUG
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME
        #endif
        };

        VkApplicationInfo applicationInfo{};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pNext = nullptr;
        applicationInfo.pApplicationName = window->Title().c_str();
        applicationInfo.applicationVersion = VK_API_VERSION_1_0;
        applicationInfo.pEngineName = "Luna3D";
        applicationInfo.engineVersion = VK_API_VERSION_1_0;
        applicationInfo.apiVersion = VK_API_VERSION_1_4;

        VkInstanceCreateInfo instanceInfo{};
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pNext = nullptr;
        instanceInfo.flags = 0;
        instanceInfo.pApplicationInfo = &applicationInfo;
        instanceInfo.enabledExtensionCount = Countof(instanceExtensions);
        instanceInfo.ppEnabledExtensionNames = instanceExtensions;
        instanceInfo.enabledLayerCount = 0;
        instanceInfo.ppEnabledLayerNames = nullptr;

    #ifdef _DEBUG
        instanceInfo.enabledLayerCount = Countof(instanceLayers);
        instanceInfo.ppEnabledLayerNames = instanceLayers;
    #endif

        VkThrowIfFailed(vkCreateInstance(&instanceInfo, nullptr, &instance));
    }
}
#pragma once

#include "Types.h"
#include "Export.h"
#include "Logger.h"
#include <vulkan/vulkan_core.h>

namespace Luna
{
    class DLL ValidationLayer
    {
    private:
        VkDebugUtilsMessengerEXT debugUtils;
        VkInstance instance;
        static Logger * logger;

    public:
        ~ValidationLayer() noexcept;

        void Initialize(VkInstance instance, Logger * logger);

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallbackUtils(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);
    };
}
#include "ValidationLayer.h"
#include "VkError.h"
#include <format>
using std::format;

namespace Luna
{
    static VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance, 
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, 
        const VkAllocationCallbacks *pAllocator, 
        VkDebugUtilsMessengerEXT *pDebugMessenger)
    {
        auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT"));
        if (func != nullptr)
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        else
            return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator)
    {
        auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (func != nullptr)
            func(instance, debugMessenger, pAllocator);
    }

    Logger* ValidationLayer::logger = nullptr;
    
    ValidationLayer::~ValidationLayer() noexcept
    {
        logger->OutputDebug(LogLevel::LOG_LEVEL_DEBUG, "Destroying Vulkan debugger");
        DestroyDebugUtilsMessengerEXT(instance, debugUtils, nullptr);
    }

    void ValidationLayer::Initialize(VkInstance instance, Logger * logger)
    {
        this->instance = instance;
        this->logger = logger;
        
        logger->OutputDebug(LogLevel::LOG_LEVEL_DEBUG, "Creating Vulkan debugger");

        VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{};
        debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugUtilsCreateInfo.messageSeverity = 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugUtilsCreateInfo.messageType = 
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugUtilsCreateInfo.pfnUserCallback = ValidationLayer::DebugCallbackUtils;
        
        VkThrowIfFailed(CreateDebugUtilsMessengerEXT(
            instance, 
            &debugUtilsCreateInfo, 
            nullptr, 
            &debugUtils));

        logger->OutputDebug(LogLevel::LOG_LEVEL_DEBUG, "Vulkan debugger created");
    }
    
    VKAPI_ATTR VkBool32 VKAPI_CALL ValidationLayer::DebugCallbackUtils(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
    {
        if (logger == nullptr)
            return VK_FALSE;

        switch (messageSeverity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                logger->OutputDebug(LogLevel::LOG_LEVEL_ERROR, format("{}", pCallbackData->pMessage));
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                logger->OutputDebug(LogLevel::LOG_LEVEL_WARN, format("{}", pCallbackData->pMessage));
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                logger->OutputDebug(LogLevel::LOG_LEVEL_INFO, format("{}", pCallbackData->pMessage));
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                logger->OutputDebug(LogLevel::LOG_LEVEL_TRACE, format("{}", pCallbackData->pMessage));
                break;
        }
        return VK_FALSE;
    }
}
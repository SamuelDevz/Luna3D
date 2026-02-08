#include "Graphics.h"
#include "VkError.h"
#include "Utils.h"
#include <vulkan/vulkan_win32.h>
#include <algorithm>
#include <format>
#include <vector>
using std::vector;
using std::format;

namespace Luna
{
    Logger Graphics::logger;

    Graphics::Graphics() noexcept
        : instance{nullptr},
        physicalDevice{nullptr}
    {
        validationlayer = new ValidationLayer();
    }

    Graphics::~Graphics() noexcept
    {
        SafeDelete(validationlayer);

        vkDestroyInstance(instance, nullptr);
    }

    static bool CheckExtensionSupported(
        const vector<VkExtensionProperties> extensions, 
        const string_view requestExtension)
    {
        return std::ranges::find_if(extensions,
            [requestExtension](auto& device_extension) {
            return strcmp(device_extension.extensionName, requestExtension.data()) == 0;
            }) != extensions.end();
    }

    static void CheckSupportMemoryBudget(
        VkPhysicalDevice gpu, 
        const vector<VkExtensionProperties> instanceExtensions, 
        const vector<VkExtensionProperties> deviceExtensions)
    {
        const bool supportMemoryBudget = 
            CheckExtensionSupported(instanceExtensions, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) &&
            CheckExtensionSupported(deviceExtensions, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);

        if (supportMemoryBudget)
        {
            VkPhysicalDeviceMemoryBudgetPropertiesEXT memoryBudgetProperties{};
            memoryBudgetProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;
            memoryBudgetProperties.pNext = nullptr;

            VkPhysicalDeviceMemoryProperties2 deviceMemoryProperties{};
            deviceMemoryProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
            deviceMemoryProperties.pNext = &memoryBudgetProperties;
            vkGetPhysicalDeviceMemoryProperties2(gpu, &deviceMemoryProperties);

            VkDeviceSize memoryTotalUsage{};
            VkDeviceSize memoryTotalBudget{};
            VkDeviceSize memoryTotalsize{};
            for (size_t i = 0; i < deviceMemoryProperties.memoryProperties.memoryHeapCount; ++i)
            {
                memoryTotalUsage += memoryBudgetProperties.heapUsage[i];
                memoryTotalBudget += memoryBudgetProperties.heapBudget[i];
                memoryTotalsize += deviceMemoryProperties.memoryProperties.memoryHeaps[i].size;
            }

            constexpr const uint32 BytesinMegaByte = 1048576U; // 1'048'576

            Graphics::logger.OutputDebug(LOG_LEVEL_INFO, 
                format("---> Video memory (size total): {}MB\n", 
                    memoryTotalsize / BytesinMegaByte)
            );

            Graphics::logger.OutputDebug(LOG_LEVEL_INFO, 
                format("---> Video memory (budget total): {}MB\n", 
                    memoryTotalBudget / BytesinMegaByte)
            );
                
            Graphics::logger.OutputDebug(LOG_LEVEL_INFO, 
                format("---> Video memory (usage total): {}MB\n", 
                    memoryTotalUsage / BytesinMegaByte)
            );
        }
    }

    void Graphics::LogHardwareInfo() const
    {
        // --------------------------------------
        // Instance Layers
        // --------------------------------------
        
        uint32 layerCount{};
        VkThrowIfFailed(vkEnumerateInstanceLayerProperties(&layerCount, nullptr))

        vector<VkLayerProperties> instanceLayers(layerCount);
        VkThrowIfFailed(vkEnumerateInstanceLayerProperties(&layerCount, instanceLayers.data()))
        
        logger.OutputDebug(LOG_LEVEL_INFO, format("---> {} Instance Layer:\n", layerCount));
        for (const auto& layer : instanceLayers)
            logger.OutputDebug(LOG_LEVEL_INFO, format("\t{}\n", layer.layerName));

        // --------------------------------------
        // Instance Extensions
        // --------------------------------------

        uint32 extensionCount{};
        VkThrowIfFailed(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr))

        vector<VkExtensionProperties> instanceExtensions(extensionCount);
        VkThrowIfFailed(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, instanceExtensions.data()))
        
        logger.OutputDebug(LOG_LEVEL_INFO, format("---> {} Instance Extensions:\n", extensionCount));
        for (const auto& extension : instanceExtensions)
            logger.OutputDebug(LOG_LEVEL_INFO, format("\t{}\n", extension.extensionName));

        // --------------------------------------
        // Device Extensions
        // --------------------------------------

        uint32 deviceExtensionCount{};
        VkThrowIfFailed(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, nullptr));
        
        vector<VkExtensionProperties> deviceExtensions(deviceExtensionCount);
        VkThrowIfFailed(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, deviceExtensions.data()))

		// --------------------------------------
		// Video adapter (GPUs)
		// --------------------------------------
		
        uint32 gpuCount{};
        vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);

        vector<VkPhysicalDevice> gpus(gpuCount);
        vkEnumeratePhysicalDevices(instance, &gpuCount, gpus.data());

        for (size_t i = 0; i < gpuCount; ++i)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(gpus[i], &deviceProperties);
            logger.OutputDebug(LOG_LEVEL_INFO, 
                format("---> Video adapter (GPU) {}: {}\n", 
                    i + 1, deviceProperties.deviceName)
            );

            CheckSupportMemoryBudget(gpus[i], instanceExtensions, deviceExtensions);

            switch (deviceProperties.deviceType)
            {
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    logger.OutputDebug(LOG_LEVEL_INFO, "---> Other\n");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    logger.OutputDebug(LOG_LEVEL_INFO, "---> Integrated GPU\n");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    logger.OutputDebug(LOG_LEVEL_INFO, "---> Discrete GPU\n");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    logger.OutputDebug(LOG_LEVEL_INFO, "---> Virtual GPU\n");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    logger.OutputDebug(LOG_LEVEL_INFO, "---> CPU\n");
                    break;
                default:
                    logger.OutputDebug(LOG_LEVEL_INFO, "---> Unknown device type\n");
                    break;
            }
            
            logger.OutputDebug(LOG_LEVEL_INFO, 
                format("---> Feature Level: {}.{}.{}\n", 
                    VK_API_VERSION_MAJOR(deviceProperties.apiVersion),
                    VK_API_VERSION_MINOR(deviceProperties.apiVersion),
                    VK_API_VERSION_PATCH(deviceProperties.apiVersion))
            );
        }

        // -----------------------------------------
        // Video Output (monitor)
        // -----------------------------------------

        DWORD deviceNum{};
        DISPLAY_DEVICE dd{};
        dd.cb = sizeof(DISPLAY_DEVICE);
        while (EnumDisplayDevices(nullptr, deviceNum, &dd, 0)) 
        {
            if (dd.StateFlags & DISPLAY_DEVICE_ACTIVE) 
                logger.OutputDebug(LOG_LEVEL_INFO, format("---> Monitor: {}\n", dd.DeviceName));
            deviceNum++;
        }

        // ------------------------------------------
		// Video mode (resolution)
		// ------------------------------------------

		uint32 dpi { GetDpiForSystem() };
		int32 screenWidth { GetSystemMetricsForDpi(SM_CXSCREEN, dpi) };
		int32 screenHeight { GetSystemMetricsForDpi(SM_CYSCREEN, dpi) };
		
		DEVMODE devMode{};
		devMode.dmSize = sizeof(DEVMODE);
		EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);
		uint32 refreshRate = devMode.dmDisplayFrequency;

		logger.OutputDebug(LOG_LEVEL_INFO, 
            format("---> Resolution: {}x{} {}Hz\n", 
                screenWidth, screenHeight, refreshRate)
        );
    }

    void Graphics::Initialize(const Window* const window)
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
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
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

    #ifdef _DEBUG
        validationlayer->Initialize(instance, &logger);
    #endif

        // ---------------------------------------------------
        // Physical Device
        // ---------------------------------------------------

        uint32 gpuCount;
        VkThrowIfFailed(vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr));

        vector<VkPhysicalDevice> gpus(gpuCount);
        VkThrowIfFailed(vkEnumeratePhysicalDevices(instance, &gpuCount, gpus.data()));
        physicalDevice = gpus[0];

    #ifdef _DEBUG
        LogHardwareInfo();
    #endif
    }
}
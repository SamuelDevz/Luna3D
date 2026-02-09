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
        : bgColor{},
        backBufferCount{},
        vSync{false},
        instance{nullptr},
        physicalDevice{nullptr},
        device{nullptr},
        surface{nullptr},
        swapchain{nullptr},
        commandBuffer{nullptr}, 
        commandPool{nullptr},
        renderPass{nullptr},
        queue{nullptr},
        fence{nullptr},
        imageAvailableSemaphore{nullptr},
        renderFinishedSemaphore{nullptr},
        viewport{},
        scissorRect{}
    {
        validationlayer = new ValidationLayer();
    }

    Graphics::~Graphics() noexcept
    {
        vkDeviceWaitIdle(device);

        vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
        vkDestroyFence(device, fence, nullptr);

        vkDestroyRenderPass(device, renderPass, nullptr);

        VkCommandBuffer commandBuffers[] { commandBuffer, copyCommandBuffer };
        vkFreeCommandBuffers(device, commandPool, 2, commandBuffers);
        vkDestroyCommandPool(device, commandPool, nullptr);

        for (uint32 i = 0; i < backBufferCount; ++i)
        {
            vkDestroyFramebuffer(device, buffers[i].framebuffer, nullptr);
            vkDestroyImageView(device, buffers[i].view, nullptr);
        }
        delete[] buffers;

        vkDestroySwapchainKHR(device, swapchain, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);

        SafeDelete(validationlayer);

        vkDestroyDevice(device, nullptr);
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

        // ---------------------------------------------------
        // Queue Family
        // ---------------------------------------------------

        uint32 queueFamilyCount{};
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

        vector<VkQueueFamilyProperties> queueProperties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueProperties.data());

        bool found = false;
        uint32 queueFamilyIndex{};
        for (uint32 i = 0; i < queueFamilyCount && !found; ++i)
        {
            if (queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                queueFamilyIndex = i;
                found = true;
            }
        }

        float queuePriorities{};
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.pNext = nullptr;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriorities;
        queueInfo.queueFamilyIndex = queueFamilyIndex;

        // ---------------------------------------------------
        // Logical Device
        // ---------------------------------------------------

        constexpr const char* deviceExtensions[]
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        VkDeviceCreateInfo deviceInfo{};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.pNext = nullptr;
        deviceInfo.queueCreateInfoCount = 1;
        deviceInfo.pQueueCreateInfos = &queueInfo;
        deviceInfo.enabledExtensionCount = Countof(deviceExtensions);
        deviceInfo.ppEnabledExtensionNames = deviceExtensions;
        deviceInfo.enabledLayerCount = 0;
        deviceInfo.ppEnabledLayerNames = nullptr;
        deviceInfo.pEnabledFeatures = nullptr;

        VkThrowIfFailed(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device));
        vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);

        // ---------------------------------------------------
        // Surface
        // ---------------------------------------------------

        VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo{};
        win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        win32SurfaceCreateInfo.pNext = nullptr;
        win32SurfaceCreateInfo.hinstance = window->AppId();
        win32SurfaceCreateInfo.hwnd = window->Id();
        VkThrowIfFailed(vkCreateWin32SurfaceKHR(instance, &win32SurfaceCreateInfo, nullptr, &surface));

        // ---------------------------------------------------
        // Swapchain
        // ---------------------------------------------------

        // Present Mode
        VkPresentModeKHR swapchainPresentMode{};

        if(vSync)
        {
            swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
        }
        else
        {
            swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

            uint32 presentModeCount{};
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

            vector<VkPresentModeKHR> presentModes(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
        
            for (uint32 i = 0; i < presentModeCount; ++i)
            {
                if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                    break;
                }
            }
        }

        // Surface Format
        uint32 surfaceFormatCount{};
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);

        vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data());

        VkSurfaceFormatKHR surfaceFormat = surfaceFormats[0];
        for (uint32 i = 0; i < surfaceFormatCount; ++i)
        {
            if (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM &&
                surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                surfaceFormat = surfaceFormats[i];
                break;
            }
        }

        // Surface Capabilities
        VkSurfaceCapabilitiesKHR surfaceCapabilities{};
        VkThrowIfFailed(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities));

        VkSurfaceTransformFlagBitsKHR preTransform{};
        if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
            preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        else
            preTransform = surfaceCapabilities.currentTransform;

        backBufferCount = surfaceCapabilities.minImageCount + 1;
        if (surfaceCapabilities.maxImageCount != 0 && backBufferCount > surfaceCapabilities.maxImageCount)
			backBufferCount = surfaceCapabilities.maxImageCount;

        // Swapchain
        VkSwapchainCreateInfoKHR swapChainCreateInfo{};
        swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.pNext = nullptr;
        swapChainCreateInfo.surface = surface;
        swapChainCreateInfo.minImageCount = backBufferCount;
        swapChainCreateInfo.imageFormat = surfaceFormat.format;
        swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapChainCreateInfo.imageExtent.width = window->Width();
        swapChainCreateInfo.imageExtent.height = window->Height();
        swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapChainCreateInfo.preTransform = preTransform;
        swapChainCreateInfo.imageArrayLayers = 1;
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.presentMode = swapchainPresentMode;
        swapChainCreateInfo.clipped = true;
        swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapChainCreateInfo.queueFamilyIndexCount = 1;
        swapChainCreateInfo.pQueueFamilyIndices = &queueFamilyIndex;

        VkThrowIfFailed(vkCreateSwapchainKHR(device, &swapChainCreateInfo, nullptr, &swapchain));

        // VkImage
        vector<VkImage> swapchainImages(backBufferCount);
        VkThrowIfFailed(vkGetSwapchainImagesKHR(device, swapchain, &backBufferCount, swapchainImages.data()));

        buffers = new SwapchainBuffer[backBufferCount] {};
        for (uint32 i = 0; i < backBufferCount; ++i)
            buffers[i].image = swapchainImages[i];

        // VkImageView
        for (uint32 i = 0; i < backBufferCount; ++i)
        {
            VkImageViewCreateInfo colorImageView{};
            colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            colorImageView.pNext = nullptr;
            colorImageView.flags = 0;
            colorImageView.image = buffers[i].image;
            colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
            colorImageView.format = surfaceFormat.format;
            colorImageView.components.r = VK_COMPONENT_SWIZZLE_R;
            colorImageView.components.g = VK_COMPONENT_SWIZZLE_G;
            colorImageView.components.b = VK_COMPONENT_SWIZZLE_B;
            colorImageView.components.a = VK_COMPONENT_SWIZZLE_A;
            colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            colorImageView.subresourceRange.baseMipLevel = 0;
            colorImageView.subresourceRange.levelCount = 1;
            colorImageView.subresourceRange.baseArrayLayer = 0;
            colorImageView.subresourceRange.layerCount = 1;

            VkThrowIfFailed(vkCreateImageView(device, &colorImageView, nullptr, &buffers[i].view));
        }

        // ---------------------------------------------------
        // Command Buffers and Command Pool
        // ---------------------------------------------------

        VkCommandPoolCreateInfo cmdPoolCreateInfo{};
        cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        cmdPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

        VkThrowIfFailed(vkCreateCommandPool(device, &cmdPoolCreateInfo, nullptr, &commandPool));
        
        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;

        VkThrowIfFailed(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer));
        VkThrowIfFailed(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &copyCommandBuffer));

        // ---------------------------------------------------
        // Renderpass
        // ---------------------------------------------------

        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = surfaceFormat.format;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentReference{};
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subPassDescription{};
        subPassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subPassDescription.colorAttachmentCount = 1;
        subPassDescription.pColorAttachments = &colorAttachmentReference;

        VkSubpassDependency subpassDependency{};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.srcAccessMask = 0;
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependency.dependencyFlags = 0;

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &attachmentDescription;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subPassDescription;
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &subpassDependency;

        VkThrowIfFailed(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass));

        // ---------------------------------------------------
        // Frame Buffers
        // ---------------------------------------------------

        for (uint32 i = 0; i < backBufferCount; ++i)
        {
            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.renderPass = renderPass;
            framebufferCreateInfo.attachmentCount = 1;
            framebufferCreateInfo.pAttachments = &buffers[i].view;
            framebufferCreateInfo.width = window->Width();
            framebufferCreateInfo.height = window->Height();
            framebufferCreateInfo.layers = 1;

            VkThrowIfFailed(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &buffers[i].framebuffer));
        }

        // ---------------------------------------------------
        // Semaphores and Fence
        // ---------------------------------------------------

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkThrowIfFailed(vkCreateFence(device, &fenceInfo, nullptr, &fence));

        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkThrowIfFailed(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore));
        VkThrowIfFailed(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphore));

        // ---------------------------------------------------
        // Viewport and Scissor Rectangle
        // ---------------------------------------------------

        viewport.x = 0;
        viewport.y = 0;
        viewport.width = static_cast<float>(window->Width());
        viewport.height = static_cast<float>(window->Height());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        scissorRect = { 0, 0, static_cast<uint32>(window->Width()), static_cast<uint32>(window->Height()) };

        // ---------------------------------------------------
        // Backbuffer Background Color
        // ---------------------------------------------------

        const COLORREF color = window->Color();

        bgColor.float32[0] = GetRValue(color) / 255.0f;
        bgColor.float32[1] = GetGValue(color) / 255.0f;
        bgColor.float32[2] = GetBValue(color) / 255.0f;
        bgColor.float32[3] = 1.0f;
    }

    void Graphics::Present()
    {
        vkWaitForFences(device, 1, &fence, true, UINT64_MAX);
        vkResetFences(device, 1, &fence);

        VkThrowIfFailed(vkAcquireNextImageKHR(
            device, 
            swapchain, 
            UINT64_MAX, 
            imageAvailableSemaphore, 
            nullptr, 
            &backBufferIndex
        ));

        VkThrowIfFailed(vkResetCommandBuffer(commandBuffer, 0));

        const VkPipelineStageFlags waitStages[]
        { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

        VkThrowIfFailed(vkQueueSubmit(queue, 1, &submitInfo, fence));

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain;
        presentInfo.pImageIndices = &backBufferIndex;

        VkThrowIfFailed(vkQueuePresentKHR(queue, &presentInfo));
    }

    static uint32 FindMemoryType(const VkPhysicalDevice physicalDevice, 
        const uint32 typeFilter, 
        const VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32 i = 0; i < memProperties.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;
        }

        return 0;
    }

    void Graphics::Allocate(const VkDeviceSize size,
        const uint32 typeFilter,
        const VkMemoryPropertyFlags properties,
        VkDeviceMemory* bufferMemory)
    {
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = size;
        allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, typeFilter, properties);

        VkThrowIfFailed(vkAllocateMemory(device, &allocInfo, nullptr, bufferMemory));
    }

    void Graphics::Allocate(const VkDeviceSize size,
        const VkBufferUsageFlags usageFlags,
        const VkMemoryPropertyFlags properties,
        VkBuffer* buffer,
        VkDeviceMemory* bufferMemory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usageFlags;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkThrowIfFailed(vkCreateBuffer(device, &bufferInfo, nullptr, buffer));

        VkMemoryRequirements memRequirements{};

        vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);
        Allocate(memRequirements.size, memRequirements.memoryTypeBits, properties, bufferMemory);

        VkThrowIfFailed(vkBindBufferMemory(device, *buffer, *bufferMemory, 0));
    }

    void Graphics::Copy(const void* vertices, const VkDeviceSize size, VkDeviceMemory bufferMemory)
    {
        void* data;
        VkThrowIfFailed(vkMapMemory(device, bufferMemory, 0, size, 0, &data));
        CopyMemory(data, vertices, size);
        vkUnmapMemory(device, bufferMemory);
    }

    void Graphics::Copy(VkBuffer destination, const VkBuffer source, const VkDeviceSize size)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VkThrowIfFailed(vkBeginCommandBuffer(copyCommandBuffer, &beginInfo));

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(copyCommandBuffer, source, destination, 1, &copyRegion);

        VkThrowIfFailed(vkEndCommandBuffer(copyCommandBuffer));

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &copyCommandBuffer;

        VkThrowIfFailed(vkQueueSubmit(queue, 1, &submitInfo, nullptr));
        VkThrowIfFailed(vkQueueWaitIdle(queue));
    }
}
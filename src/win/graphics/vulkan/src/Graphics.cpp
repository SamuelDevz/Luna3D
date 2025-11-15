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
        : backBufferCount{2},
        vSync{false},
        bgColor{},
        instance{nullptr},
        physicalDevice{nullptr},
        device{nullptr},
        surface{nullptr},
        swapchain{nullptr},
        backBufferIndex{},
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
        buffers = new SwapchainBuffer[backBufferCount] {};
        validationlayer = new ValidationLayer();
    }

    Graphics::~Graphics() noexcept
    {
        vkDeviceWaitIdle(device);

        vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
        vkDestroyFence(device, fence, nullptr);

        for (uint32 i = 0; i < backBufferCount; ++i)
        {
            vkDestroyFramebuffer(device, buffers[i].framebuffer, nullptr);
            vkDestroyImageView(device, buffers[i].view, nullptr);
        }

        delete[] buffers;

        VkCommandBuffer commandBuffers[]{ commandBuffer };
        vkFreeCommandBuffers(device, commandPool, 1, commandBuffers);
        vkDestroyCommandPool(device, commandPool, nullptr);

        vkDestroyRenderPass(device, renderPass, nullptr);

        vkDestroySwapchainKHR(device, swapchain, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);

        SafeDelete(validationlayer);
        
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    static bool CheckExtensionSupported(const vector<VkExtensionProperties> extensions, const string_view requestExtension)
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

    void Graphics::LogHardwareInfo()
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

        VkPresentModeKHR swapchainPresentMode{};

        if(vSync)
        {
            swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
        }
        else
        {
            swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

            uint32_t presentModeCount{};
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

            vector<VkPresentModeKHR> presentModes(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
        
            for (uint32_t i = 0; i < presentModeCount; ++i)
            {
                if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                    break;
                }
            }
        }

        VkSurfaceCapabilitiesKHR surfCapabilities{};
        VkThrowIfFailed(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfCapabilities));

        VkSurfaceTransformFlagBitsKHR preTransform{};
        if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
            preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        else
            preTransform = surfCapabilities.currentTransform;

        VkSwapchainCreateInfoKHR swapChainCreateInfo{};
        swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.pNext = nullptr;
        swapChainCreateInfo.surface = surface;
        swapChainCreateInfo.minImageCount = backBufferCount;
        swapChainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
        swapChainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
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

        vector<VkImage> swapchainImages(backBufferCount);
        VkThrowIfFailed(vkGetSwapchainImagesKHR(device, swapchain, &backBufferCount, swapchainImages.data()));

        for (uint32 i = 0; i < backBufferCount; ++i)
            buffers[i].image = swapchainImages[i];

        for (uint32 i = 0; i < backBufferCount; ++i)
        {
            VkImageViewCreateInfo colorImageView{};
            colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            colorImageView.pNext = nullptr;
            colorImageView.flags = 0;
            colorImageView.image = buffers[i].image;
            colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
            colorImageView.format = VK_FORMAT_B8G8R8A8_UNORM;
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
        // Command Buffer and Command Pool
        // ---------------------------------------------------

        VkCommandPoolCreateInfo cmdPoolCreateInfo{};
        cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        cmdPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

        VkThrowIfFailed(vkCreateCommandPool(device, &cmdPoolCreateInfo, nullptr, &commandPool));

        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.commandBufferCount = 1;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        VkThrowIfFailed(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer));

        // ---------------------------------------------------
        // Renderpass
        // ---------------------------------------------------

        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = VK_FORMAT_B8G8R8A8_UNORM;
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

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &attachmentDescription;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subPassDescription;

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

    void Graphics::ResetCommands() const noexcept
    {
        vkWaitForFences(device, 1, &fence, true, UINT64_MAX);
        vkResetFences(device, 1, &fence);
        vkResetCommandBuffer(commandBuffer, 0);
    }

    void Graphics::BeginCommandRecording() const
    {
        ResetCommands();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkClearValue clearValue{};
        clearValue.color = bgColor;

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = buffers[backBufferIndex].framebuffer;
        renderPassInfo.renderArea = scissorRect;
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValue;

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void Graphics::EndCommandRecording() const
    {
        vkCmdEndRenderPass(commandBuffer);
        VkThrowIfFailed(vkEndCommandBuffer(commandBuffer));
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

        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

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
    
    static uint32 FindMemoryType(VkPhysicalDevice physicalDevice, 
        uint32 typeFilter, 
        VkMemoryPropertyFlags properties)
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

    void Graphics::Allocate(VkDeviceSize size,
        uint32 typeFilter,
        VkMemoryPropertyFlags properties,
        VkDeviceMemory* bufferMemory)
    {
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = size;
        allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, typeFilter, properties);

        VkThrowIfFailed(vkAllocateMemory(device, &allocInfo, nullptr, bufferMemory));
    }

    void Graphics::Allocate(VkDeviceSize size,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags properties,
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

    void Graphics::Copy(const void* vertices, VkDeviceSize size, VkDeviceMemory bufferMemory)
    {
        void* data;
        VkThrowIfFailed(vkMapMemory(device, bufferMemory, 0, size, 0, &data));
        CopyMemory(data, vertices, size);
        vkUnmapMemory(device, bufferMemory);
    }

    void Graphics::Copy(VkBuffer destination, VkBuffer source, VkDeviceSize size)
    {
        ResetCommands();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VkThrowIfFailed(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        VkBufferCopy copyRegion{};
        copyRegion.size = size;

        vkCmdCopyBuffer(commandBuffer, source, destination, 1, &copyRegion);

        VkThrowIfFailed(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkThrowIfFailed(vkQueueSubmit(queue, 1, &submitInfo, nullptr));
        VkThrowIfFailed(vkQueueWaitIdle(queue));
    }
}
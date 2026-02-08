#pragma once

#include "Types.h"
#include "Export.h"
#include "Logger.h"
#include "Window.h"
#include "ValidationLayer.h"
#include <vulkan/vulkan.h>

namespace Luna
{
    struct SwapchainBuffer
    {
        VkImage image;
        VkImageView view;
        VkFramebuffer framebuffer;
    };

    class DLL Graphics
    {
    private:
        // config
        uint32                       backBufferCount;
        bool                         vSync;
        VkClearColorValue            bgColor;

        // pipeline
        VkInstance                   instance;
        VkPhysicalDevice             physicalDevice;
        VkDevice                     device;

        VkSurfaceKHR                 surface;
        VkSwapchainKHR               swapchain;
        SwapchainBuffer            * buffers;

        VkCommandBuffer              commandBuffer;
        VkCommandBuffer              copyCommandBuffer;
        VkCommandPool                commandPool;

        VkRenderPass                 renderPass;

        // synchronization
        VkQueue                      queue;
        VkSemaphore                  imageAvailableSemaphore;
        VkSemaphore                  renderFinishedSemaphore;
        VkFence                      fence;

        void LogHardwareInfo() const;

        ValidationLayer * validationlayer;
        
    public:
        static Logger logger;
        
        VkViewport                   viewport;
        VkRect2D                     scissorRect;

        explicit Graphics() noexcept;
        ~Graphics() noexcept;

        void VSync(const bool state) noexcept;
        void Initialize(const Window * const window);

        VkPhysicalDevice PhysicalDevice() const noexcept;
        VkDevice Device() const noexcept;
        VkCommandBuffer CommandBuffer() const noexcept;
        VkRenderPass RenderPass() const noexcept;
    };

    inline void Graphics::VSync(const bool state) noexcept
    { vSync = state; }

    inline VkPhysicalDevice Graphics::PhysicalDevice() const noexcept
    { return physicalDevice; }

    inline VkDevice Graphics::Device() const noexcept
    { return device; }

    inline VkCommandBuffer Graphics::CommandBuffer() const noexcept
    { return commandBuffer; }

    inline VkRenderPass Graphics::RenderPass() const noexcept
    { return renderPass; }
};
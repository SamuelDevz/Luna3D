#pragma once

#include "Types.h"
#include "Export.h"
#include "Window.h"
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
        uint32                       backBufferIndex;

        VkCommandBuffer              commandBuffer;
        VkCommandPool                commandPool;

        VkRenderPass                 renderPass;
        VkQueue                      queue;

        // synchronization
        VkSemaphore                  imageAvailableSemaphore;
        VkSemaphore                  renderFinishedSemaphore;
        VkFence                      fence;

        bool WaitCommandQueue() noexcept;

    public:
        explicit Graphics() noexcept;
        ~Graphics() noexcept;

        VkViewport                   viewport;
        VkRect2D                     scissorRect;

        void VSync(const bool state) noexcept;
        void Initialize(const Window* const window);
        void Present();

        void ResetCommands() const noexcept;
        void BeginCommandRecording() const;
        void EndCommandRecording() const;

        void Allocate(VkDeviceSize size,
            uint32 typeFilter,
            VkMemoryPropertyFlags properties,
            VkDeviceMemory* bufferMemory);

        void Allocate(VkDeviceSize size,
            VkBufferUsageFlags usageFlags,
            VkMemoryPropertyFlags properties,
            VkBuffer* buffer,
            VkDeviceMemory* bufferMemory);

        void Copy(const void* vertices,
            VkDeviceSize size,
            VkDeviceMemory bufferMemory);

        void Copy(VkBuffer destination,
            VkBuffer source,
            VkDeviceSize size);

        VkDevice Device() const noexcept;
        VkPhysicalDevice PhysicalDevice() const noexcept;
        VkRenderPass RenderPass() const noexcept;
        VkCommandBuffer CommandBuffer() const noexcept;
    };

    inline void Graphics::VSync(const bool state) noexcept
    { vSync = state; }

    inline VkDevice Graphics::Device() const noexcept
    { return device; }

    inline VkPhysicalDevice Graphics::PhysicalDevice() const noexcept
    { return physicalDevice; }

    inline VkRenderPass Graphics::RenderPass() const noexcept
    { return renderPass; }

    inline VkCommandBuffer Graphics::CommandBuffer() const noexcept
    { return commandBuffer; }
}
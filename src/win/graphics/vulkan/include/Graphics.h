#pragma once

#include "Export.h"
#include "Window.h"
#include "Types.h"
#include <vulkan/vulkan.h>

namespace Luna
{
    class DLL Graphics
    {
    private:
        // pipeline
        VkInstance                   instance;
        VkPhysicalDevice             physicalDevice;

    public:
        explicit Graphics() noexcept;
        ~Graphics() noexcept;

        void Initialize(const Window * const window);

        VkPhysicalDevice PhysicalDevice() const noexcept;
    };

    inline VkPhysicalDevice Graphics::PhysicalDevice() const noexcept
    { return physicalDevice; }
};
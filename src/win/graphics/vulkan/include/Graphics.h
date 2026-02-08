#pragma once

#include "Types.h"
#include "Export.h"
#include "Logger.h"
#include "Window.h"
#include "ValidationLayer.h"
#include <vulkan/vulkan.h>

namespace Luna
{
    class DLL Graphics
    {
    private:
        VkInstance                   instance;
        VkPhysicalDevice             physicalDevice;

        ValidationLayer * validationlayer;
        
    public:
        static Logger logger;
        
        explicit Graphics() noexcept;
        ~Graphics() noexcept;

        void Initialize(const Window * const window);

        VkPhysicalDevice PhysicalDevice() const noexcept;
    };

    inline VkPhysicalDevice Graphics::PhysicalDevice() const noexcept
    { return physicalDevice; }
};
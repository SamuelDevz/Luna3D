#pragma once

#include "Types.h"
#include "Export.h"
#include "Window.h"
#include "Logger.h"
#include <vulkan/vulkan.h>

namespace Luna
{
    class DLL Graphics
    {
    private:
        // pipeline
        VkInstance                   instance;

    public:
        static Logger logger;

        explicit Graphics() noexcept;
        ~Graphics() noexcept;

        void Initialize(const Window * const window);
    };
};
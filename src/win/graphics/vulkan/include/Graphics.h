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

    public:
        explicit Graphics() noexcept;
        ~Graphics() noexcept;

        void Initialize(const Window * const window);
    };
};
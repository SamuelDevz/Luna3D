#pragma once

#include "Types.h"
#include "Export.h"
#include "Window.h"
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
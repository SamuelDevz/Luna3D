#pragma once

#include "Types.h"
#include <vulkan/vulkan_core.h>

namespace Luna
{
    struct StagingBuffer
    {
        VkBuffer buffer;
        VkDeviceMemory memory;
    };
    
    struct Mesh
    {
        string id;

        int32 vertexCount;
        uint32 vertexBufferSize;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;

        StagingBuffer vertexUploadBuffer;

	    VkDevice device;

        Mesh(const string_view name) noexcept;
        ~Mesh() noexcept;
    };
}
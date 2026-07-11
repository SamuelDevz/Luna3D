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
        StagingBuffer vertexUploadBuffer;

        uint32 vertexBufferSize;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
	    VkDevice device;

		int32 vertexCount;

        Mesh(const string_view name) noexcept;
        ~Mesh() noexcept;
    };
}
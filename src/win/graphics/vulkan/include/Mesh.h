#pragma once

#include "Types.h"
#include <vulkan/vulkan_core.h>

namespace Luna
{
    struct Mesh
    {
        string id;

        int32 vertexCount;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;

        VkBuffer vertexBufferUpload;
        VkDeviceMemory vertexBufferUploadMemory;

	    VkDevice device;

        Mesh(const string_view name) noexcept;
        ~Mesh() noexcept;
    };      
}
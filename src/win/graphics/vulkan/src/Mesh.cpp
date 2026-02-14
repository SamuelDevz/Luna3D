#include "Mesh.h"

namespace Luna
{
    Mesh::Mesh(const string_view name) noexcept
        : id{name},
        vertexCount{},
        vertexBufferSize{},
        vertexUploadBuffer{nullptr, nullptr},
        vertexBuffer{nullptr},
        vertexBufferMemory{nullptr},
        device{nullptr}
    {
    }
    
    Mesh::~Mesh() noexcept
    {
        if (device)
        {
            if (vertexBuffer)
                vkDestroyBuffer(device, vertexBuffer, nullptr);
            
            if (vertexBufferMemory)
                vkFreeMemory(device, vertexBufferMemory, nullptr);
            
            if (vertexUploadBuffer.buffer)
                vkDestroyBuffer(device, vertexUploadBuffer.buffer, nullptr);
            
            if (vertexUploadBuffer.memory)
                vkFreeMemory(device, vertexUploadBuffer.memory, nullptr);
        }
    }
}
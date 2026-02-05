#include "Mesh.h"

namespace Luna
{
    Mesh::Mesh(const string_view name) noexcept
        : id{name},
        vertexCount{},
        vertexBuffer{nullptr},
        vertexBufferMemory{nullptr},
        vertexBufferUpload{nullptr},
        vertexBufferUploadMemory{nullptr},
        device{nullptr}
    {
    }
    
    Mesh::~Mesh() noexcept
    {
        vkDestroyBuffer(device, vertexBuffer, nullptr);
	    vkFreeMemory(device, vertexBufferMemory, nullptr);
    }
}
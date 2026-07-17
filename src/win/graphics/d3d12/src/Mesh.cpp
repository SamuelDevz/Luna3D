#include "Mesh.h"
#include "Utils.h"

namespace Luna
{
    Mesh::Mesh(const string_view name) noexcept
        : id{name},
        vertexBufferCPU{nullptr},
        vertexBufferGPU{nullptr},
        vertexBufferUpload{nullptr},
        vertexByteStride{},
        vertexBufferSize{}
    {
    }
    
    Mesh::~Mesh() noexcept
    {
        SafeRelease(vertexBufferUpload);
        SafeRelease(vertexBufferGPU);
        SafeRelease(vertexBufferCPU);
    }
    
    D3D12_VERTEX_BUFFER_VIEW* Mesh::VertexBufferView() noexcept
    {
        vertexBufferView.BufferLocation = vertexBufferGPU->GetGPUVirtualAddress();
        vertexBufferView.StrideInBytes = vertexByteStride;
        vertexBufferView.SizeInBytes = vertexBufferSize;
    
        return &vertexBufferView;
    }
}
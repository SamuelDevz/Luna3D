#pragma once

#include <d3d12.h>
#include "Types.h"

namespace Luna
{
    struct Mesh
    {
        string id;

        ID3DBlob* vertexBufferCPU;
        ID3D12Resource* vertexBufferUpload;
        ID3D12Resource* vertexBufferGPU;

        D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

        uint32 vertexByteStride;
        uint32 vertexBufferSize;

        Mesh(const string_view name) noexcept;
        ~Mesh() noexcept;
        
        D3D12_VERTEX_BUFFER_VIEW * VertexBufferView() noexcept;
    };
}

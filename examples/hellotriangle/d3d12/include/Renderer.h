#pragma once

#include "All.h"
#include <d3dcommon.h>
#include <DirectXMath.h>
using namespace DirectX;

namespace Luna
{
    using Position = XMFLOAT3;
    using Color = XMFLOAT4;

    struct Vertex
    {
        Position pos;
        Color color;
    };
    
    class Renderer final
    {
    private:
        Graphics                * graphics;
        ID3D12RootSignature     * rootSignature;
        ID3D12PipelineState     * pipelineState;
        
    public:
        explicit Renderer() noexcept;
        ~Renderer() noexcept;

        void Initialize(Graphics * graphics);
        void Draw() noexcept;
        void Clear() noexcept;
    };
    
    inline void Renderer::Clear() noexcept
    { graphics->Clear(pipelineState); }
    
    inline void Renderer::Draw() noexcept
    { 
        graphics->CommandList()->SetGraphicsRootSignature(rootSignature); 
        graphics->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }
}
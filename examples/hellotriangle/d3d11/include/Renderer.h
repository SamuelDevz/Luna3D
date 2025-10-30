#pragma once

#include "All.h"
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
        ID3D11InputLayout       * inputLayout;
        ID3D11VertexShader      * vertexShader;
        ID3D11PixelShader       * pixelShader;
        ID3D11RasterizerState   * rasterState;
        ID3D11Buffer            * vertexBuffer;

	public:
        explicit Renderer() noexcept;
        ~Renderer() noexcept;

        void Initialize(Graphics * graphics, const void * vertices, const uint32 points);
        void Draw();
	};
}
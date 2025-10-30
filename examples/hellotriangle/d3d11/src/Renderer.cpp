#include "Renderer.h"
#include <source_location>
#include <d3dcompiler.h>
#include <filesystem>

namespace Luna
{
    namespace fs = std::filesystem;

    Renderer::Renderer() noexcept 
        : graphics{nullptr},
        inputLayout{nullptr},
        vertexShader{nullptr},
        pixelShader{nullptr},
        rasterState{nullptr},
        vertexBuffer{nullptr}
	{
	}

	Renderer::~Renderer() noexcept
	{
        SafeRelease(inputLayout);
        SafeRelease(vertexShader);
        SafeRelease(pixelShader);
        SafeRelease(rasterState);
        SafeRelease(vertexBuffer);
	}

    HRESULT CompileShaderFromFile(const wstring_view file,
        const string_view entryPoint,
        const string_view shaderModel,
        ID3DBlob ** shaderBlob)
    {
        uint32 flags = D3DCOMPILE_ENABLE_STRICTNESS;

    #ifdef _DEBUG
        flags |= D3DCOMPILE_DEBUG;
        flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
    #elif NDEBUG
        flags |= D3DCOMPILE_OPTIMIZATION_LEVEL2;
    #endif

        return D3DCompileFromFile(
            file.data(),
            nullptr,
            nullptr,
            entryPoint.data(),
            shaderModel.data(),
            flags,
            0,
            shaderBlob,
            nullptr);
    }

	void Renderer::Initialize(Graphics * graphics, const void * vertices, const uint32 points)
	{
		this->graphics = graphics;

        const std::source_location& location = std::source_location::current();
        const std::filesystem::path baseDir = std::filesystem::path(location.file_name()).parent_path();
        wstring vertexPath = baseDir.wstring().substr(0, baseDir.string().size() - 3);
        vertexPath += L"/resources/Shaders/";

        //-------------------------------
        // Vertex Shader
        //-------------------------------

        ID3DBlob* vShader = nullptr;
        wstring vertexFile = vertexPath + L"Vertex.hlsl";
        ThrowIfFailed(CompileShaderFromFile(vertexFile.c_str(), "main", "vs_5_0", &vShader))
        ThrowIfFailed(graphics->Device()->CreateVertexShader(
            vShader->GetBufferPointer(),
            vShader->GetBufferSize(),
            nullptr,
            &vertexShader))

        //-------------------------------
        // Pixel Shader
        //-------------------------------

        ID3DBlob* pShader = nullptr;
        wstring pixelFile = vertexPath + L"Pixel.hlsl";
        ThrowIfFailed(CompileShaderFromFile(pixelFile.c_str(), "main", "ps_5_0", &pShader))
        ThrowIfFailed(graphics->Device()->CreatePixelShader(
            pShader->GetBufferPointer(),
            pShader->GetBufferSize(),
            nullptr,
            &pixelShader))

        //-------------------------------
        // Input Layout
        //-------------------------------

        constexpr D3D11_INPUT_ELEMENT_DESC layoutDesc[]
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        ThrowIfFailed(graphics->Device()->CreateInputLayout(
            layoutDesc,
            Countof(layoutDesc),
            vShader->GetBufferPointer(),
            vShader->GetBufferSize(),
            &inputLayout))

        //-------------------------------
        // Rasterizador
        //-------------------------------

        D3D11_RASTERIZER_DESC rasterDesc{};
        rasterDesc.FillMode = D3D11_FILL_SOLID;
        //rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
        rasterDesc.CullMode = D3D11_CULL_BACK;
        rasterDesc.DepthClipEnable = true;
        rasterDesc.FrontCounterClockwise = false;
        rasterDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
        rasterDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
        rasterDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rasterDesc.DepthClipEnable = true;
        rasterDesc.MultisampleEnable = false;
        rasterDesc.AntialiasedLineEnable = false;

        ThrowIfFailed(graphics->Device()->CreateRasterizerState(&rasterDesc, &rasterState))

        //-------------------------------
        // Vertex Buffer
        //-------------------------------

        D3D11_BUFFER_DESC vertexBufferDesc{};
        vertexBufferDesc.ByteWidth = sizeof(Vertex) * points;
        vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        D3D11_SUBRESOURCE_DATA vertexData{};
        vertexData.pSysMem = vertices;

        ThrowIfFailed(graphics->Device()->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer))

        Draw();

        SafeRelease(vShader);
        SafeRelease(pShader);
	}

    void Renderer::Draw()
    {
        uint32 vertexStride = sizeof(Vertex);
        uint32 vertexOffset{};
        graphics->Context()->IASetInputLayout(inputLayout);
        graphics->Context()->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &vertexOffset);
        graphics->Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        graphics->Context()->VSSetShader(vertexShader, nullptr, 0);
        graphics->Context()->PSSetShader(pixelShader, nullptr, 0);
        graphics->Context()->RSSetState(rasterState);
    }
}
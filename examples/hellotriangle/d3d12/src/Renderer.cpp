#include "Renderer.h"
#include <source_location>
#include <filesystem>
#include <d3dcompiler.h>

namespace Luna
{
    namespace fs = std::filesystem;

    Renderer::Renderer() noexcept
        : graphics{nullptr},
        rootSignature{nullptr},
        pipelineState{nullptr}
	{
	}

	Renderer::~Renderer() noexcept
	{
	    SafeRelease(rootSignature);
        SafeRelease(pipelineState);
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

	void Renderer::Initialize(Graphics * graphics)
	{
		this->graphics = graphics;
		
		// -----------------------------------------------------------
        // Root signature
        // -----------------------------------------------------------
        
        D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
        rootSigDesc.NumParameters = 0;
        rootSigDesc.pParameters = nullptr;
        rootSigDesc.NumStaticSamplers = 0;
        rootSigDesc.pStaticSamplers = nullptr;
        rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ID3DBlob* serializedRootSig = nullptr;
        ID3DBlob* error = nullptr;

        ThrowIfFailed(D3D12SerializeRootSignature(
            &rootSigDesc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            &serializedRootSig,
            &error));

        ThrowIfFailed(graphics->Device()->CreateRootSignature(
            0,
            serializedRootSig->GetBufferPointer(),
            serializedRootSig->GetBufferSize(),
            IID_PPV_ARGS(&rootSignature)));

		// -----------------------------------------------------------
        // Pipeline state
        // -----------------------------------------------------------

        // --------------------
        // --- Input Layout ---
        // --------------------

        D3D12_INPUT_ELEMENT_DESC inputLayout[2]
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // --------------------
        // ----- Shaders ------
        // --------------------

        ID3DBlob* vertexShader;
        ID3DBlob* pixelShader;

        const std::source_location& location = std::source_location::current();
        const std::filesystem::path baseDir = std::filesystem::path(location.file_name()).parent_path();
        wstring vertexPath = baseDir.wstring().substr(0, baseDir.string().size() - 3);
        vertexPath += L"/resources/Shaders/";

        wstring vertexFile = vertexPath + L"Vertex.hlsl";
        ThrowIfFailed(CompileShaderFromFile(vertexFile.c_str(), "main", "vs_5_0", &vertexShader))

        wstring pixelFile = vertexPath + L"Pixel.hlsl";
        ThrowIfFailed(CompileShaderFromFile(pixelFile.c_str(), "main", "ps_5_0", &pixelShader))

        // --------------------
        // ---- Rasterizer ----
        // --------------------

        D3D12_RASTERIZER_DESC rasterizer{};
        rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
        //rasterizer.FillMode = D3D12_FILL_MODE_WIREFRAME;
        rasterizer.CullMode = D3D12_CULL_MODE_BACK;
        rasterizer.FrontCounterClockwise = false;
        rasterizer.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        rasterizer.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        rasterizer.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rasterizer.DepthClipEnable = true;
        rasterizer.MultisampleEnable = false;
        rasterizer.AntialiasedLineEnable = false;
        rasterizer.ForcedSampleCount = 0;
        rasterizer.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        // ---------------------
        // --- Color Blender ---
        // ---------------------

        D3D12_BLEND_DESC blender{};
        blender.AlphaToCoverageEnable = FALSE;
        blender.IndependentBlendEnable = FALSE;
        const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
        {
            FALSE,FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL,
        };
        for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
            blender.RenderTarget[i] = defaultRenderTargetBlendDesc;

        // -----------------------------------
        // --- Pipeline State Object (PSO) ---
        // -----------------------------------

        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso{};
        pso.pRootSignature = rootSignature;
        pso.VS = { reinterpret_cast<BYTE*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
        pso.PS = { reinterpret_cast<BYTE*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
        pso.BlendState = blender;
        pso.SampleMask = UINT_MAX;
        pso.RasterizerState = rasterizer;
        pso.InputLayout = { inputLayout, 2 };
        pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso.NumRenderTargets = 1;
        pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        pso.SampleDesc.Count = graphics->Antialiasing();
        pso.SampleDesc.Quality = graphics->Quality();
        graphics->Device()->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&pipelineState));

        SafeRelease(vertexShader);
        SafeRelease(pixelShader);
	}
}
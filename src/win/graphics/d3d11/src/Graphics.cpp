#include "Graphics.h"
#include "Error.h"
#include "Utils.h"
#include <format>
using std::format;

namespace Luna
{
    Graphics::Graphics() noexcept
        : device{nullptr},
        context{nullptr},
        factory{nullptr},
        swapChain{nullptr},
        renderTargetView{nullptr},
        blendState{nullptr},
        featureLevel{D3D_FEATURE_LEVEL_11_1},
        bgColor{},
        vSync{false},
        viewport{},
        backBufferCount{2},
        antialiasing{1},
        quality{},
        refreshRate{},
        scissorRect{}
    {
    }

    Graphics::~Graphics() noexcept
    {
        SafeRelease(device);
        SafeRelease(factory);
        SafeRelease(renderTargetView);
        SafeRelease(blendState);

        if (swapChain)
        {
            swapChain->SetFullscreenState(false, nullptr);
            swapChain->Release();
            swapChain = nullptr;
        }

        if (context)
        {
            context->ClearState();
            context->Release();
            context = nullptr;
        }
    }

    void Graphics::Initialize(const Window * const window)
    {
        // ---------------------------------------------------
        // DXGI infrastructure and D3D device
        // ---------------------------------------------------

        uint32 createDeviceFlags{};
        uint32 createFactoryFlags{};

    #ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        createFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    #endif

        constexpr D3D_FEATURE_LEVEL featureLevels[]
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };

        ThrowIfFailed(D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            featureLevels,
            Countof(featureLevels),
            D3D11_SDK_VERSION,
            reinterpret_cast<ID3D11Device**>(&device),
            &featureLevel,
            reinterpret_cast<ID3D11DeviceContext**>(&context)));

        ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory)));

        // ---------------------------------------------------
        // Swap Chain
        // ---------------------------------------------------

        IDXGIFactory2 * dxgiFactory = nullptr;
        factory->QueryInterface(IID_PPV_ARGS(&dxgiFactory));

        if(dxgiFactory)
        {
            // DirectX 11.1 or later
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
            swapChainDesc.Width = window->Width();
            swapChainDesc.Height = window->Height();
            swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swapChainDesc.SampleDesc.Count = antialiasing;
            swapChainDesc.SampleDesc.Quality = quality;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = backBufferCount;
            swapChainDesc.Scaling = DXGI_SCALING_NONE;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

            DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDesc{};
            swapChainFullscreenDesc.RefreshRate.Numerator = refreshRate;
            swapChainFullscreenDesc.RefreshRate.Denominator = 1;
            swapChainFullscreenDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
            swapChainFullscreenDesc.Windowed = (window->Mode() != FULLSCREEN);

            ThrowIfFailed(factory->CreateSwapChainForHwnd(
                device,
                window->Id(),
                &swapChainDesc,
                &swapChainFullscreenDesc,
                nullptr,
                &swapChain));
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC swapChainDesc{};
            swapChainDesc.BufferDesc.Width = uint32(window->Width());
            swapChainDesc.BufferDesc.Height = uint32(window->Height());
            swapChainDesc.BufferDesc.RefreshRate.Numerator = refreshRate;
            swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
            swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swapChainDesc.SampleDesc.Count = antialiasing;
            swapChainDesc.SampleDesc.Quality = quality;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = backBufferCount;
            swapChainDesc.OutputWindow = window->Id();
            swapChainDesc.Windowed = (window->Mode() != FULLSCREEN);
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

            ThrowIfFailed(factory->CreateSwapChain(
                device,
                &swapChainDesc,
                reinterpret_cast<IDXGISwapChain**>(&swapChain)
            ))
        }

        ThrowIfFailed(swapChain->GetParent(IID_PPV_ARGS(&factory)));
        ThrowIfFailed(factory->MakeWindowAssociation(window->Id(), DXGI_MWA_NO_ALT_ENTER));

        // ---------------------------------------------------
        // Render Target View
        // ---------------------------------------------------

        ID3D11Texture2D * backBuffer = nullptr;
        ThrowIfFailed(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
        ThrowIfFailed(device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView));

        context->OMSetRenderTargets(1, &renderTargetView, nullptr);

        // ---------------------------------------------------
        // Viewport and Scissor Rectangle
        // ---------------------------------------------------

        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = static_cast<float>(window->Width());
        viewport.Height = static_cast<float>(window->Height());
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        context->RSSetViewports(1, &viewport);

        scissorRect = { 0, 0, window->Width(), window->Height() };

        context->RSSetScissorRects(1, &scissorRect);

        // ---------------------------------------------
        // Blend State
        // ---------------------------------------------

        // Color blending equation:
        // finalColor = SrcColor * SrcBlend <OP> DestColor * DestBlend

        // Combining transparent surfaces (Alpha Blending)
        // finalColor = SrcColor * ScrAlpha + DestColor * (1-SrcAlpha)

        D3D11_BLEND_DESC blendDesc{};
        blendDesc.AlphaToCoverageEnable = false;
        blendDesc.IndependentBlendEnable = false;
        constexpr D3D11_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc
        {
            false,
            D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD,
            D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD,
            D3D11_COLOR_WRITE_ENABLE_ALL,
        };
        for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
            blendDesc.RenderTarget[i] = defaultRenderTargetBlendDesc;

        ThrowIfFailed(device->CreateBlendState(&blendDesc, &blendState))

        context->OMSetBlendState(blendState, nullptr, 0xffffffff);

        // ---------------------------------------------------
        // Backbuffer Background Color
        // ---------------------------------------------------

        const COLORREF color = window->Color();

        bgColor[0] = GetRValue(color) / 255.0f;
        bgColor[1] = GetGValue(color) / 255.0f;
        bgColor[2] = GetBValue(color) / 255.0f;
        bgColor[3] = 1.0f;

        SafeRelease(backBuffer);
        SafeRelease(dxgiFactory);
    }
}

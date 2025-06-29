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
        depthStencilState{nullptr},
        depthStencilView{nullptr},
        depthStencil{nullptr},
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
        debugLayer = new DebugLayer();
    }

    Graphics::~Graphics() noexcept 
    {
        SafeDelete(debugLayer);

        SafeRelease(device);
        SafeRelease(factory);
        SafeRelease(renderTargetView);
        SafeRelease(depthStencilState);
        SafeRelease(depthStencilView);
        SafeRelease(depthStencil);
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

    void Graphics::LogHardwareInfo() noexcept
    {
        const uint32 BytesinMegaByte = 1048576U; // 1'048'576 

        // --------------------------------------
        // Video adapter (GPU)
        // --------------------------------------
        IDXGIAdapter* adapter = nullptr;
        if (factory->EnumAdapters(0, &adapter) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_ADAPTER_DESC desc;
            adapter->GetDesc(&desc);
            OutputDebugStringW(format(L"---> Video adapter GPU: {}\n", desc.Description).c_str());
        }

        IDXGIAdapter4* adapter4 = nullptr;
        if (SUCCEEDED(adapter->QueryInterface(IID_PPV_ARGS(&adapter4))))
        {
            DXGI_QUERY_VIDEO_MEMORY_INFO memInfo;
            adapter4->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo);
            
            OutputDebugStringW(format(L"---> Video memory (free): {}MB\n", memInfo.Budget / BytesinMegaByte).c_str());
            OutputDebugStringW(format(L"---> Video memory (used): {}MB\n", memInfo.CurrentUsage / BytesinMegaByte).c_str());

            adapter4->Release();
        }

        // -----------------------------------------
        // Maximum Feature Level supported by GPU
        // -----------------------------------------

        {
            wstring text = L"---> Feature Level: ";
            switch (featureLevel)
            {
                case D3D_FEATURE_LEVEL_11_1: text += L"11_1\n"; break;
                case D3D_FEATURE_LEVEL_11_0: text += L"11_0\n"; break;
                case D3D_FEATURE_LEVEL_10_1: text += L"10_1\n"; break;
                case D3D_FEATURE_LEVEL_10_0: text += L"10_0\n"; break;
                case D3D_FEATURE_LEVEL_9_3:  text += L"9_3\n";  break;
                case D3D_FEATURE_LEVEL_9_2:  text += L"9_2\n";  break;
                case D3D_FEATURE_LEVEL_9_1:  text += L"9_1\n";  break;
            }
            OutputDebugStringW(text.c_str());
        }

        // -----------------------------------------
        // Video output (monitor)
        // -----------------------------------------

        IDXGIOutput* output = nullptr;
        if (adapter->EnumOutputs(0, &output) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_OUTPUT_DESC desc;
            output->GetDesc(&desc);
            OutputDebugStringW(format(L"---> Monitor: {}\n", desc.DeviceName).c_str());
        }

        // ------------------------------------------
        // Video mode (resolution)
        // ------------------------------------------

        uint32 dpi { GetDpiForSystem() };
        int32 screenWidth { GetSystemMetricsForDpi(SM_CXSCREEN, dpi) }; 
        int32 screenHeight { GetSystemMetricsForDpi(SM_CYSCREEN, dpi) };

        OutputDebugStringW(
            format(L"---> Resolution: {}x{} {}Hz\n",
                screenWidth, screenHeight, refreshRate).c_str()
        );

        SafeRelease(adapter);
        SafeRelease(output);
    }

    void Graphics::RefreshRate() noexcept
    {
        DEVMODE devMode{};
        devMode.dmSize = sizeof(DEVMODE);
        EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);
        refreshRate = devMode.dmDisplayFrequency;
    }

    void Graphics::Initialize(const Window * const window)
    {
        // ---------------------------------------------------
        // DXGI infrastructure and D3D device
        // ---------------------------------------------------

        uint32 createDeviceFlags{};

    #ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
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

        debugLayer->InitDebugLayer(reinterpret_cast<IUnknown*>(device));
        debugLayer->CreateFactory(IID_PPV_ARGS(&factory));

        RefreshRate();

    #ifdef _DEBUG
        LogHardwareInfo();
    #endif 

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
                reinterpret_cast<IDXGISwapChain**>(&swapChain)));
        }

        ThrowIfFailed(swapChain->GetParent(IID_PPV_ARGS(&factory)));
        ThrowIfFailed(factory->MakeWindowAssociation(window->Id(), DXGI_MWA_NO_ALT_ENTER));

        // ---------------------------------------------------
        // Render Target View
        // ---------------------------------------------------

        ID3D11Texture2D * backBuffer = nullptr;
        ThrowIfFailed(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
        ThrowIfFailed(device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView));

        // ---------------------------------------------------
        // Depth Stencil View
        // ---------------------------------------------------
        
        D3D11_TEXTURE2D_DESC depthStencilDesc{};
        depthStencilDesc.Width = window->Width();
        depthStencilDesc.Height = window->Height();
        depthStencilDesc.MipLevels = 1;
        depthStencilDesc.ArraySize = 1;
        depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesc.SampleDesc.Count = antialiasing;
        depthStencilDesc.SampleDesc.Quality = quality;
        depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
        depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthStencilDesc.CPUAccessFlags = 0;
        depthStencilDesc.MiscFlags = 0;
            
        ThrowIfFailed(device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencil))

        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
        depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Texture2D.MipSlice = 0;
            
        ThrowIfFailed(device->CreateDepthStencilView(depthStencil, &depthStencilViewDesc, &depthStencilView))
            
        context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

        // ---------------------------------------------------
        // Depth Stencil State
        // ---------------------------------------------------
        
        {
            D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
            depthStencilDesc.DepthEnable = TRUE;
            depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
            depthStencilDesc.StencilEnable = FALSE;
            depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
            depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
            constexpr D3D11_DEPTH_STENCILOP_DESC defaultStencilOp
            { D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS };
            depthStencilDesc.FrontFace = defaultStencilOp;
            depthStencilDesc.BackFace = defaultStencilOp;
            
            ThrowIfFailed(device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState));

            context->OMSetDepthStencilState(depthStencilState, 1);
        }
       
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

        COLORREF color = window->Color();

        bgColor[0] = GetRValue(color)/255.0f;
        bgColor[1] = GetGValue(color)/255.0f;
        bgColor[2] = GetBValue(color)/255.0f;
        bgColor[3] = 1.0f;

        SafeRelease(backBuffer);
        SafeRelease(dxgiFactory);
    }
}
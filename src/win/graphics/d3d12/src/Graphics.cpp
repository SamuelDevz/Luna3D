#include "Graphics.h"
#include "Error.h"
#include "Utils.h"
#include <format>
using std::format;

namespace Luna
{
    Logger Graphics::logger;

    Graphics::Graphics() noexcept
        : refreshRate{},
        backBufferCount{2},
        backBufferIndex{},
        antialiasing{1},
        quality{},
        vSync{false},
        bgColor{},
        device{nullptr},
        factory{nullptr},
        swapChain{nullptr},
        commandQueue{nullptr},
        commandList{nullptr},
        commandListAlloc{nullptr},
        renderTargetHeap{nullptr},
        rtDescriptorSize{},
        viewport{},
        scissorRect{},
        featureLevel{D3D_FEATURE_LEVEL_11_0},
        fence{nullptr},
        currentFence{}
    {
        renderTargets = new ID3D12Resource*[backBufferCount] {nullptr};
        debugLayer = new DebugLayer(logger);
    }

    Graphics::~Graphics() noexcept
    {
        SafeDelete(debugLayer);
        
        WaitCommandQueue();

        if (renderTargets)
        {
            for (uint32 i = 0; i < backBufferCount; ++i)
                SafeRelease(renderTargets[i]);
            delete[] renderTargets;
        }

        if (swapChain)
        {
            swapChain->SetFullscreenState(false, nullptr);
            swapChain->Release();
        }

        SafeRelease(fence);
        SafeRelease(renderTargetHeap);
    	SafeRelease(commandList);
     	SafeRelease(commandListAlloc);
     	SafeRelease(commandQueue);
     	SafeRelease(device);
     	SafeRelease(factory);
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
            logger.OutputDebug(LOG_LEVEL_INFO, format(L"---> Video adapter GPU: {}\n", desc.Description).c_str());
        }

        IDXGIAdapter4* adapter4 = nullptr;
        if (SUCCEEDED(adapter->QueryInterface(IID_PPV_ARGS(&adapter4))))
        {
            DXGI_QUERY_VIDEO_MEMORY_INFO memInfo;
            adapter4->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo);

            logger.OutputDebug(LOG_LEVEL_INFO, format("---> Video memory (free): {}MB\n", memInfo.Budget / BytesinMegaByte).c_str());
            logger.OutputDebug(LOG_LEVEL_INFO, format("---> Video memory (used): {}MB\n", memInfo.CurrentUsage / BytesinMegaByte).c_str());

            adapter4->Release();
        }

        // -----------------------------------------
        // Maximum Feature Level supported by GPU
        // -----------------------------------------
        constexpr D3D_FEATURE_LEVEL featureLevels[10]
        {
            D3D_FEATURE_LEVEL_12_2,
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };

        D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevelsInfo;
        featureLevelsInfo.NumFeatureLevels = 10;
        featureLevelsInfo.pFeatureLevelsRequested = featureLevels;

        device->CheckFeatureSupport(
            D3D12_FEATURE_FEATURE_LEVELS,
            &featureLevelsInfo,
            sizeof(featureLevelsInfo));

        {
            string text = "---> Feature Level: ";
            switch (featureLevelsInfo.MaxSupportedFeatureLevel)
            {
                case D3D_FEATURE_LEVEL_12_2: text += "12_2\n"; break;
                case D3D_FEATURE_LEVEL_12_1: text += "12_1\n"; break;
                case D3D_FEATURE_LEVEL_12_0: text += "12_0\n"; break;
                case D3D_FEATURE_LEVEL_11_1: text += "11_1\n"; break;
                case D3D_FEATURE_LEVEL_11_0: text += "11_0\n"; break;
                case D3D_FEATURE_LEVEL_10_1: text += "10_1\n"; break;
                case D3D_FEATURE_LEVEL_10_0: text += "10_0\n"; break;
                case D3D_FEATURE_LEVEL_9_3:  text += "9_3\n";  break;
                case D3D_FEATURE_LEVEL_9_2:  text += "9_2\n";  break;
                case D3D_FEATURE_LEVEL_9_1:  text += "9_1\n";  break;
            }
            logger.OutputDebug(LOG_LEVEL_INFO, text.c_str());
        }

        // -----------------------------------------
        // Video output (monitor)
        // -----------------------------------------

        IDXGIOutput* output = nullptr;
        if (adapter->EnumOutputs(0, &output) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_OUTPUT_DESC desc;
            output->GetDesc(&desc);
            logger.OutputDebug(LOG_LEVEL_INFO, format(L"---> Monitor: {}\n", desc.DeviceName).c_str());
        }

        // ------------------------------------------
        // Video mode (resolution)
        // ------------------------------------------

        uint32 dpi { GetDpiForSystem() };
        int32 screenWidth { GetSystemMetricsForDpi(SM_CXSCREEN, dpi) };
        int32 screenHeight { GetSystemMetricsForDpi(SM_CYSCREEN, dpi) };

        logger.OutputDebug(LOG_LEVEL_INFO,
            format("---> Resolution: {}x{} {}Hz\n",
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

        uint32 factoryFlags{};
        
        debugLayer->InitDebugLayer();
        debugLayer->CreateFactory(IID_PPV_ARGS(&factory));

    	if FAILED(D3D12CreateDevice(
    		nullptr,
    		D3D_FEATURE_LEVEL_11_0,
    		IID_PPV_ARGS(&device)))
    	{
    		IDXGIAdapter* warp;
    		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warp)));
    		ThrowIfFailed(D3D12CreateDevice(
    			warp,
    			D3D_FEATURE_LEVEL_11_0,
    			IID_PPV_ARGS(&device)));

    		warp->Release();

            logger.OutputDebug(LOG_LEVEL_DEBUG, "---> Using WARP Adapter: D3D12 is not supported.\n");
    	}

        RefreshRate();

    #ifdef _DEBUG
        LogHardwareInfo();
    #endif

        // ---------------------------------------------------
    	// Create queue, commandlist and commandListAlloc
    	// ---------------------------------------------------

    	D3D12_COMMAND_QUEUE_DESC queueDesc{};
    	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    	ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

    	ThrowIfFailed(device->CreateCommandAllocator(
    		D3D12_COMMAND_LIST_TYPE_DIRECT,
    		IID_PPV_ARGS(&commandListAlloc)));

    	ThrowIfFailed(device->CreateCommandList(
    		0,
    		D3D12_COMMAND_LIST_TYPE_DIRECT,
    		commandListAlloc,
    		nullptr,
    		IID_PPV_ARGS(&commandList)));

        // ---------------------------------------------------
    	// Creates the fence to synchronize CPU/GPU.
    	// ---------------------------------------------------

    	ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

        // ---------------------------------------------------
        // Swap Chain
        // ---------------------------------------------------

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
            commandQueue,
            window->Id(),
            &swapChainDesc,
            &swapChainFullscreenDesc,
            nullptr,
            reinterpret_cast<IDXGISwapChain1**>(&swapChain)));

        ThrowIfFailed(factory->MakeWindowAssociation(window->Id(), DXGI_MWA_NO_ALT_ENTER));

        // ---------------------------------------------------
        // Render Target Views (and heaps associated)
        // ---------------------------------------------------

        D3D12_DESCRIPTOR_HEAP_DESC renderTargetHeapDesc{};
        renderTargetHeapDesc.NumDescriptors = backBufferCount;
        renderTargetHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        renderTargetHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(device->CreateDescriptorHeap(&renderTargetHeapDesc, IID_PPV_ARGS(&renderTargetHeap)));

        D3D12_CPU_DESCRIPTOR_HANDLE rtHandle = renderTargetHeap->GetCPUDescriptorHandleForHeapStart();

        rtDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        for (uint32 i = 0; i < backBufferCount; ++i)
        {
            swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
            device->CreateRenderTargetView(renderTargets[i], nullptr, rtHandle);
            rtHandle.ptr += rtDescriptorSize;
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

        commandList->RSSetViewports(1, &viewport);

        scissorRect = { 0, 0, window->Width(), window->Height() };

        commandList->RSSetScissorRects(1, &scissorRect);

        // ---------------------------------------------------
        // Backbuffer Background Color
        // ---------------------------------------------------

        const COLORREF color = window->Color();

        bgColor[0] = GetRValue(color) / 255.0f;
        bgColor[1] = GetGValue(color) / 255.0f;
        bgColor[2] = GetBValue(color) / 255.0f;
        bgColor[3] = 1.0f;
    }

    void Graphics::Clear(ID3D12PipelineState * pso) noexcept
    {
        commandListAlloc->Reset();
        commandList->Reset(commandListAlloc, pso);

        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = renderTargets[backBufferIndex];
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        commandList->ResourceBarrier(1, &barrier);

        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &scissorRect);

        D3D12_CPU_DESCRIPTOR_HANDLE rtHandle = renderTargetHeap->GetCPUDescriptorHandleForHeapStart();
        rtHandle.ptr += SIZE_T(backBufferIndex) * SIZE_T(rtDescriptorSize);
        commandList->ClearRenderTargetView(rtHandle, bgColor, 0, nullptr);

        commandList->OMSetRenderTargets(1, &rtHandle, false, nullptr);
    }

    bool Graphics::WaitCommandQueue() noexcept
    {
    	currentFence++;

    	if (FAILED(commandQueue->Signal(fence, currentFence)))
    		return false;

    	if (fence->GetCompletedValue() < currentFence)
    	{
    		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

    		if (eventHandle)
    		{
    			if (FAILED(fence->SetEventOnCompletion(currentFence, eventHandle)))
    				return false;

    			WaitForSingleObject(eventHandle, INFINITE);
    			CloseHandle(eventHandle);
    		}
    	}

    	return true;
    }

    void Graphics::SubmitCommands() noexcept
    {
    	commandList->Close();
    	ID3D12CommandList* cmdsLists[] = { commandList };
    	commandQueue->ExecuteCommandLists(Countof(cmdsLists), cmdsLists);

    	WaitCommandQueue();
    }

    void Graphics::Present() noexcept
    {
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = renderTargets[backBufferIndex];
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        commandList->ResourceBarrier(1, &barrier);

        SubmitCommands();

        swapChain->Present(vSync, 0);
        backBufferIndex = (backBufferIndex + 1) % backBufferCount;
    }

    void Graphics::Allocate(const uint32 type,
        const uint32 sizeInBytes,
        ID3D12Resource** resource) const
    {
        D3D12_HEAP_PROPERTIES bufferProp{};
        bufferProp.Type = D3D12_HEAP_TYPE_DEFAULT;
        bufferProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        bufferProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        bufferProp.CreationNodeMask = 1;
        bufferProp.VisibleNodeMask = 1;

        if (type == UPLOAD)
            bufferProp.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC bufferDesc{};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Alignment = 0;
        bufferDesc.Width = sizeInBytes;
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.SampleDesc.Quality = 0;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON;

        if (type == UPLOAD)
            initState = D3D12_RESOURCE_STATE_GENERIC_READ;

        ThrowIfFailed(device->CreateCommittedResource(
            &bufferProp,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            initState,
            nullptr,
            IID_PPV_ARGS(resource)));
    }

    void Graphics::Copy(const void* vertices,
        uint32 sizeInBytes,
        ID3D12Resource* bufferUpload,
        ID3D12Resource* bufferGPU) const
    {
        // ----------------------------------------------------------------------------------
        // Copies vertices to the default buffer (GPU)
        // ----------------------------------------------------------------------------------
        //
        // To copy data to the GPU:
        // - first copy the data to the intermediate upload heap
        // - then using ID3D12CommandList::CopyBufferRegion copy from upload to the GPU
        // ----------------------------------------------------------------------------------

        D3D12_SUBRESOURCE_DATA vertexSubResourceData{};
        vertexSubResourceData.pData = vertices;
        vertexSubResourceData.RowPitch = sizeInBytes;
        vertexSubResourceData.SlicePitch = sizeInBytes;

        D3D12_PLACED_SUBRESOURCE_FOOTPRINT layouts{};
        uint32 numRows{};
        uint64 rowSizesInBytes{};
        uint64 requiredSize{};

        D3D12_RESOURCE_DESC bufferGPUDesc = bufferGPU->GetDesc();

        device->GetCopyableFootprints(
            &bufferGPUDesc,
            0, 1, 0, &layouts, &numRows,
            &rowSizesInBytes, &requiredSize);

        BYTE* pData;
        bufferUpload->Map(0, nullptr, reinterpret_cast<void**>(&pData));

        D3D12_MEMCPY_DEST DestData
        {
            pData + layouts.Offset,
            layouts.Footprint.RowPitch,
            layouts.Footprint.RowPitch * uint64(numRows)
        };

        for (uint32 z = 0; z < layouts.Footprint.Depth; ++z)
        {
            BYTE * destSlice = reinterpret_cast<BYTE*>(DestData.pData) + DestData.SlicePitch * z;

            const BYTE* srcSlice = reinterpret_cast<const BYTE*>(vertexSubResourceData.pData) + vertexSubResourceData.SlicePitch * z;

            for (uint32 y = 0; y < numRows; ++y)
            {
                memcpy(destSlice + DestData.RowPitch * y,
                       srcSlice + vertexSubResourceData.RowPitch * y,
                       static_cast<size_t>(rowSizesInBytes));
            }
        }

        bufferUpload->Unmap(0, nullptr);

        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = bufferGPU;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        commandList->ResourceBarrier(1, &barrier);

        commandList->CopyBufferRegion(
            bufferGPU,
            0,
            bufferUpload,
            layouts.Offset,
            layouts.Footprint.Width);

        barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = bufferGPU;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        commandList->ResourceBarrier(1, &barrier);
    }
}
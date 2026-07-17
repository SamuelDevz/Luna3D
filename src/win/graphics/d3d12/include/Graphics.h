#pragma once

#include <dxgi1_4.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include "Export.h"
#include "Window.h"
#include "Types.h"
#include <d3dcompiler.h>

namespace Luna
{
    enum AllocationType { GPU, UPLOAD };

    class DLL Graphics
    {
    private:
        // config
        uint32                       refreshRate;
        uint32                       backBufferCount;
        uint32                       antialiasing;
        uint32                       quality;
        bool                         vSync;
        float                        bgColor[4];

        // pipeline
        ID3D12Device4              * device;
        IDXGIFactory6              * factory;
        IDXGISwapChain3            * swapChain;
        uint32                       backBufferIndex;

        ID3D12CommandQueue         * commandQueue;
        ID3D12GraphicsCommandList  * commandList;
        ID3D12CommandAllocator     * commandListAlloc;

        ID3D12Resource            ** renderTargets;
        ID3D12DescriptorHeap       * renderTargetHeap;
        uint32                       rtDescriptorSize;
        D3D_FEATURE_LEVEL            featureLevel;
        D3D12_VIEWPORT               viewport;
        D3D12_RECT                   scissorRect;

        // sincronization
        ID3D12Fence                * fence;
        uint64                       currentFence;

        bool WaitCommandQueue() noexcept;

    public:
        explicit Graphics() noexcept;
        ~Graphics() noexcept;

        void VSync(const bool state) noexcept;
        void Initialize(const Window * const window);
        void Clear(ID3D12PipelineState * pso) noexcept;
        void Present() noexcept;

        void ResetCommands() noexcept;
        void SubmitCommands() noexcept;

        void Allocate(uint32 sizeInBytes,
            ID3DBlob** resource) const noexcept;

        void Allocate(const uint32 type,
            const uint32 sizeInBytes,
            ID3D12Resource** resource) const;

        void Copy(const void* vertices,
            uint32 sizeInBytes,
            ID3DBlob* bufferCPU) const noexcept;

        void Copy(const void* vertices,
            uint32 sizeInBytes,
            ID3D12Resource* bufferUpload,
            ID3D12Resource* bufferGPU) const;

        ID3D12Device4* Device() const noexcept;
        ID3D12GraphicsCommandList* CommandList() const noexcept;
        uint32 Antialiasing() const noexcept;
        uint32 Quality() const noexcept;
    };

    inline void Graphics::VSync(const bool state) noexcept
    { vSync = state; }

    inline ID3D12Device4* Graphics::Device() const noexcept
    { return device; }

    inline ID3D12GraphicsCommandList* Graphics::CommandList() const noexcept
    { return commandList; }

    inline uint32 Graphics::Antialiasing() const noexcept
    { return antialiasing; }

    inline uint32 Graphics::Quality() const noexcept
    { return quality; }

    inline void Graphics::ResetCommands() noexcept
    { commandList->Reset(commandListAlloc, nullptr); }

    inline void Graphics::Allocate(uint32 sizeInBytes, ID3DBlob** resource) const noexcept
    { D3DCreateBlob(sizeInBytes, resource); }

    inline void Graphics::Copy(const void* vertices, uint32 sizeInBytes, ID3DBlob* bufferCPU) const noexcept
    { CopyMemory(bufferCPU->GetBufferPointer(), vertices, sizeInBytes); }
};
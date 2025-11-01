#pragma once

#include "Types.h"
#include "Logger.h"
#include <d3d12sdklayers.h>
#include <dxgidebug.h>

namespace Luna
{
    class DebugLayer
    {
    private:
        ID3D12Debug5 * debugController;
        ID3D12DebugDevice* debugDevice;
        IDXGIDebug1 * dxgiDebug;
        IDXGIInfoQueue * dxgiInfoQueue;
        
        uint32 dxgiFactoryFlags;
        
        Logger & logger;

    public:
        explicit DebugLayer(Logger & logger) noexcept;
        ~DebugLayer() noexcept;

        void InitDebugLayer();
        void CreateFactory(REFIID riid, void ** ppFactory);
    };
}
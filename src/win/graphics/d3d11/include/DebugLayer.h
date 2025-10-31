#pragma once

#include "Types.h"
#include <d3d11sdklayers.h>
#include <dxgidebug.h>

namespace Luna
{
    class DebugLayer
    {
    private:
        ID3D11Debug * debugController;
        ID3D11InfoQueue * infoQueue;
        IDXGIDebug1 * dxgiDebug;
        IDXGIInfoQueue * dxgiInfoQueue;
    
    public:
        explicit DebugLayer() noexcept;
        ~DebugLayer() noexcept;

        void InitDebugLayer(IUnknown * const device) noexcept;
        void CreateFactory(REFIID riid, void ** ppFactory);
    };
}
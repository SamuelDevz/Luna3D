#include "DebugLayer.h"
#include "Error.h"
#include "Utils.h"
#include <dxgi1_3.h>

namespace Luna
{
    DebugLayer::DebugLayer(Logger & logger) noexcept
        : debugController{nullptr},
        dxgiDebug{nullptr},
        dxgiInfoQueue{nullptr},
        logger{logger},
        dxgiFactoryFlags{}
    {
    #if defined(_DEBUG)
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
            dxgiDebug->EnableLeakTrackingForThread();

        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue))))
        {
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
        }
        
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    #endif
    }

    DebugLayer::~DebugLayer() noexcept
    {
    #if defined(_DEBUG)
        if (dxgiDebug)
        {
            logger.OutputDebug(LOG_LEVEL_DEBUG, "DXGI Reports living device objects:\n");
            dxgiDebug->ReportLiveObjects(
                DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_DETAIL)
            );
        }

        SafeRelease(debugController);
        SafeRelease(dxgiInfoQueue);
        SafeRelease(dxgiDebug);
    #endif
    }

    void DebugLayer::InitDebugLayer()
    {
    #ifdef _DEBUG
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))
        debugController->EnableDebugLayer();
    #endif
    }

    void DebugLayer::CreateFactory(REFIID riid, void **ppFactory)
    {
        ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, riid, ppFactory));
        
    #if defined(_DEBUG)
        dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
        dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

        DXGI_INFO_QUEUE_MESSAGE_ID hide[]
        {
            80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
        };
        DXGI_INFO_QUEUE_FILTER filter{};
        filter.DenyList.NumIDs = Countof(hide);
        filter.DenyList.pIDList = hide;
        dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);        
    #endif
    }
}
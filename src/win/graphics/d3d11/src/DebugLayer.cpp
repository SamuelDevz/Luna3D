#include "DebugLayer.h"
#include "Error.h"
#include "Utils.h"
#include <dxgi1_3.h>

namespace Luna
{
    DebugLayer::DebugLayer() noexcept
        : debugController{nullptr},
        infoQueue{nullptr},
        dxgiDebug{nullptr},
        dxgiInfoQueue{nullptr}
    {
    #if defined(_DEBUG)
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
            dxgiDebug->EnableLeakTrackingForThread();

        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue))))
        {
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
        }
    #endif
    }

    DebugLayer::~DebugLayer() noexcept
    {
    #if defined(_DEBUG)
        debugController->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL));

        if (dxgiDebug)
        {
            OutputDebugStringW(L"DXGI Reports living device objects:\n");
            dxgiDebug->ReportLiveObjects(
                DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL)
            );
        }

        SafeRelease(debugController);
        SafeRelease(dxgiInfoQueue);
        SafeRelease(infoQueue);
        SafeRelease(dxgiDebug);
    #endif
    }

    void DebugLayer::InitDebugLayer(IUnknown * const device) noexcept
    {
    #if !defined(NDEBUG)
        if (SUCCEEDED(device->QueryInterface(&debugController)))
        {
            if (SUCCEEDED(dxgiDebug->QueryInterface(&infoQueue)))
            {
            #if defined(_DEBUG)
                infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
                infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
            #endif

                D3D11_MESSAGE_ID hide[]
                {
                    D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                    // Add more message IDs here as needed
                };
                D3D11_INFO_QUEUE_FILTER filter = {};
                filter.DenyList.NumIDs = Countof(hide);
                filter.DenyList.pIDList = hide;
                infoQueue->AddStorageFilterEntries(&filter);
            }
        }
    #endif
    }

    void DebugLayer::CreateFactory(REFIID riid, void **ppFactory)
    {
    #if defined(_DEBUG)
        ThrowIfFailed(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, riid, ppFactory));

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
    #else
        ThrowIfFailed(CreateDXGIFactory2(0, riid, ppFactory));
    #endif
    }
}
#include "Input.h"
#include "KeyCodes.h"
#include <windowsx.h>

namespace Luna
{
    bool Input::keys[MAX_KEYS] = {};
    bool Input::ctrl[MAX_KEYS] = {};
    string Input::text;

    int32 Input::mouseX = 0;
    int32 Input::mouseY = 0;
    int16 Input::mouseWheel = 0;

    Input::Input() noexcept
    {
        SetWindowLongPtr(GetActiveWindow(), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Input::InputProc));
    }

    Input::~Input() noexcept
    {
        SetWindowLongPtr(GetActiveWindow(), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::WinProc));
    }

    bool Input::KeyPress(const int32 vkcode) noexcept
    {
        if (ctrl[vkcode])
        {
            if (KeyDown(vkcode))
            {
                ctrl[vkcode] = false;
                return true;
            }
        }
        else if (KeyUp(vkcode))
        {
            ctrl[vkcode] = true;
        }

        return false;
    }

    void Input::Read() noexcept
    {
        text.clear();
        SetWindowLongPtr(GetActiveWindow(), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Input::Reader));
    }

    int16 Input::MouseWheel() noexcept
    {
        int16 val = mouseWheel;
        mouseWheel = 0;
        return val;
    }

    LRESULT CALLBACK Input::Reader(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (msg == WM_CHAR)
        {
            switch (wParam)
            {
            case VK_BACK:
                if (!text.empty())
                    text.erase(text.size() - 1);
                break;

            case VK_TAB:
            case VK_RETURN:
                SetWindowLongPtr(GetActiveWindow(), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Input::InputProc));
                break;
            
            default:
                text += char(wParam);
                break;
            }

            return 0;
        }

        return CallWindowProc(Input::InputProc, hWnd, msg, wParam, lParam);
    }

    LRESULT CALLBACK Input::InputProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_KEYDOWN:
            keys[wParam] = true;
            return 0;

        case WM_KEYUP:
            keys[wParam] = false;
            return 0;
        
        case WM_MOUSEMOVE:			
            mouseX = GET_X_LPARAM(lParam);
            mouseY = GET_Y_LPARAM(lParam);
            return 0;
        
        case WM_MOUSEWHEEL:
            mouseWheel = GET_WHEEL_DELTA_WPARAM(wParam);
            return 0;

        case WM_LBUTTONDOWN:		
        case WM_LBUTTONDBLCLK:
            keys[VK_LBUTTON] = true;
            return 0;
        
        case WM_MBUTTONDOWN:		
        case WM_MBUTTONDBLCLK:
            keys[VK_MBUTTON] = true;
            return 0;
        
        case WM_RBUTTONDOWN:		
        case WM_RBUTTONDBLCLK:
            keys[VK_RBUTTON] = true;
            return 0;
        
        case WM_LBUTTONUP:			
            keys[VK_LBUTTON] = false;
            return 0;
        
        case WM_MBUTTONUP:			
            keys[VK_MBUTTON] = false;
            return 0;
        
        case WM_RBUTTONUP:			
            keys[VK_RBUTTON] = false;
            return 0;
        }

        return CallWindowProc(Window::WinProc, hWnd, msg, wParam, lParam);
    }
}
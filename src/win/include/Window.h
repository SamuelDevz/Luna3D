#pragma once

#include "Types.h"
#include "WinInclude.h"
#include "Export.h"
#include <windows.h>

namespace Luna
{
    enum WindowModes { FULLSCREEN, WINDOWED, BORDERLESS };

    class DLL Window
    {
    private:
        HWND		windowHandle;
        HINSTANCE   hInstance;
        int32		windowWidth;
        int32		windowHeight;
        HICON		windowIcon;
        HCURSOR		windowCursor;
        COLORREF	windowColor;
        string		windowTitle;
        DWORD		windowStyle;
        int32		windowMode;
        int32		windowPosX;
        int32		windowPosY;
        int32		windowCenterX;
        int32		windowCenterY;

        static void (*inFocus)();
        static void (*lostFocus)();
        
    public:
        explicit Window() noexcept;

        HWND Id() const noexcept;
        HINSTANCE AppId() const noexcept;
        int32 Width() const noexcept;
        int32 Height() const noexcept;
        int32 Mode() const noexcept;
        int32 CenterX() const noexcept;
        int32 CenterY() const noexcept;
        string Title() const noexcept;
        COLORREF Color() const noexcept;
        float AspectRatio() const noexcept;

        void Icon(const uint32 icon) noexcept;
        void Cursor(const uint32 cursor) noexcept;
        void Title(const string_view title) noexcept;
        void Size(const uint32 width, const uint32 height) noexcept;
        void Mode(const uint32 mode) noexcept;
        void Color(const uint8 r, const uint8 g, const uint8 b) noexcept;

        void HideCursor(const bool hide) const noexcept;
        void Close() const noexcept;
        bool Create() noexcept;

        void InFocus(void(*func)()) noexcept;
        void LostFocus(void(*func)()) noexcept;

        static LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };

    inline HINSTANCE Window::AppId() const noexcept
    { return hInstance; }

    inline HWND Window::Id() const noexcept
    { return windowHandle; }

    inline int32 Window::Width() const noexcept
    { return windowWidth; }

    inline int32 Window::Height() const noexcept
    { return windowHeight; }

    inline int32 Window::Mode() const noexcept
    { return windowMode; }

    inline int32 Window::CenterX() const noexcept
    { return windowCenterX; }

    inline int32 Window::CenterY() const noexcept
    { return windowCenterY; }

    inline string Window::Title() const noexcept
    { return windowTitle; }

    inline COLORREF Window::Color() const noexcept
    { return windowColor; }

    inline float Window::AspectRatio() const noexcept
    { return windowWidth / float(windowHeight); }

    inline void Window::Icon(const uint32 icon) noexcept
    { windowIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(icon)); }

    inline void Window::Cursor(const uint32 cursor) noexcept
    { windowCursor = LoadCursor(GetModuleHandle(nullptr), MAKEINTRESOURCE(cursor)); }

    inline void Window::Title(const string_view title) noexcept
    { windowTitle = title; }

    inline void Window::Color(const uint8 r, const uint8 g, const uint8 b) noexcept
    { windowColor = RGB(r,g,b); }

    inline void Window::HideCursor(const bool hide) const noexcept
    { ShowCursor(!hide); }

    inline void Window::Close() const noexcept
    { PostMessage(windowHandle, WM_DESTROY, 0, 0); }

    inline void Window::InFocus(void(*func)()) noexcept
    { inFocus = func; }

    inline void Window::LostFocus(void(*func)()) noexcept
    { lostFocus = func; }
}
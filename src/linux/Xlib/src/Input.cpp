#include "Input.h"
#include "KeyCodes.h"
#include <X11/Xlocale.h>

namespace Luna
{
    Display * Input::display = nullptr;
    XWindow Input::window = 0;
    XEvent * Input::event = nullptr;

    bool Input::keys[MAX_KEYS] = {};
    bool Input::ctrl[MAX_KEYS] = {};
    string Input::text;

    int32 Input::mouseX = 0;
    int32 Input::mouseY = 0;
    int16 Input::mouseWheel = 0;

    XIM Input::xim = nullptr;
    XIC Input::xic = nullptr;

    Input::~Input() noexcept
    {
        XUnsetICFocus(xic);
        if (xic) XDestroyIC(xic);
		if (xim) XCloseIM(xim);
    }

    void Input::Initialize(Display * display, XWindow window, XEvent * event)
    {
        this->display = display;
        this->window = window;
        this->event = event;

        setlocale(LC_ALL, "");
        XSupportsLocale();
        XSetLocaleModifiers("@im=none");

        xim = XOpenIM(display, nullptr, nullptr, nullptr);
        xic = XCreateIC(
            xim,
            XNInputStyle,
            XIMPreeditNothing | XIMStatusNothing,
            XNClientWindow,
            window, nullptr
        );

        XSetICFocus(xic);
    }

    bool Input::XKeyPress(const uint32 vkcode) noexcept
    {
        auto keycode = XKeysymToKeycode(display, vkcode);

        if (ctrl[keycode])
        {
            if (KeyDown(vkcode))
            {
                ctrl[keycode] = false;
                return true;
            }
        }
        else if (KeyUp(vkcode))
        {
            ctrl[keycode] = true;
        }

        return false;
    }

    void Input::Read() noexcept
    {
        text.clear();
        Input::Reader(event);
    }

    int16 Input::MouseWheel() noexcept
    {
        int16 val = mouseWheel;
        mouseWheel = 0;
        return val;
    }

    string LookupText(XIC xic, XKeyEvent * keyboard) 
    {
        string buffer;
        buffer.resize(2);
        KeySym ks{};
        Status status;

        int length = XmbLookupString(xic, keyboard, &buffer[0], buffer.size(), nullptr, &status);
        buffer.resize(length);
        length = XmbLookupString(xic, keyboard, &buffer[0], buffer.size(), nullptr, &status);

        return buffer;
    }

    void Input::Reader(XEvent * event)
    {
        bool read = true;
        while(read)
        {
            XNextEvent(display, event);
            if (XFilterEvent(event, window))
                continue;

            switch(event->type)
            {
            case MappingNotify:
                XRefreshKeyboardMapping(&event->xmapping);
                break;
                
            case KeyPress:
                KeySym key = XLookupKeysym(&event->xkey, 0);

                switch (key)
                {
                case VK_BACK:
                    if (!text.empty())
                        text.erase(text.size() - 1);
                    break;

                case VK_TAB:
                    Input::InputProc(event);
                    break;
                
                case VK_RETURN:
                    read = false;
                    break;
                }

                text += LookupText(xic, &event->xkey);
                break;
            }
        }

        Input::InputProc(event);
    }

    void Input::InputProc(XEvent * event)
    {
        switch(event->type)
        {
        case KeyPress:
            keys[event->xkey.keycode] = true;
            break;
        
        case KeyRelease:
            keys[event->xkey.keycode] = false;
            break;
        
        case MotionNotify:
            mouseX = event->xmotion.x;
            mouseY = event->xmotion.y;
            break;
        
        case ButtonPress:
            switch(event->xbutton.button)
            {
            case VK_LBUTTON:
                keys[VK_LBUTTON] = true;
                break;

            case VK_MBUTTON:
                keys[VK_MBUTTON] = true;
                break;

            case VK_RBUTTON:
                keys[VK_RBUTTON] = true;
                break;

            case VK_SCROLL_UP:
                mouseWheel = 120;
                break;

            case VK_SCROLL_DOWN:
                mouseWheel = -120;
                break;
            }
            break;

        case ButtonRelease:
            switch(event->xbutton.button)
            {
            case VK_LBUTTON:
                keys[VK_LBUTTON] = false;
                break;

            case VK_MBUTTON:
                keys[VK_MBUTTON] = false;
                break;

            case VK_RBUTTON:
                keys[VK_RBUTTON] = false;
                break;
            }
            break;
        }
    }
}
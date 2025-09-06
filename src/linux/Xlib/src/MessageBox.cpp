// SPDX-License-Identifier: LGPL-3.0-or-later
/*
   I, David Oberhollenzer, author of this file hereby place the contents of
   this file into the public domain. Please feel free to use this file in any
   way you wish.
   I want to do this, because a lot of people are in the need of a simple X11
   message box function.
   The original version was written in C++ and has been ported to C. This
   version is entirely leak proof! (According to valgrind)
*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <cstring>

/**************************************************************************
* A "small" and "simple" function that creates a message box with an OK  *
* button, using ONLY Xlib.                                               *
* The function does not return until the user closes the message box,    *
* using the OK button, the escape key, or the close button what means    *
* that you can't do anything in the mean time(in the same thread).       *
* The code may look very ugly, because I pieced it together from         *
* tutorials and manuals and I use an awfull lot of magic values and      *
* unexplained calculations.                                              *
*                                                                        *
* title: The title of the message box.                                   *
* text:  The contents of the message box. Use '\n' as a line terminator. *
**************************************************************************/

void MessageBox(const char * title, const char * text)
{
    Display* display = XOpenDisplay(nullptr);
    if (!display) 
        return;

    /* Get us a white and black color */
    const unsigned long black = BlackPixel(display, DefaultScreen(display));
    const unsigned long white = WhitePixel(display, DefaultScreen(display));
    XColor color{};
    const char * grey = "#dcdad5";
    XParseColor(display, DefaultColormap(display, 0), grey, &color);
    XAllocColor(display, DefaultColormap(display, 0), &color);

    /* Create a window with the specified title */
    Window window = XCreateSimpleWindow(
        display, DefaultRootWindow(display), 
        0, 0,
        100, 100,
        0,
        black, color.pixel);

    XStoreName(display, window, title);

    XSelectInput(display, window, ExposureMask
        | StructureNotifyMask 
        | KeyReleaseMask 
        | PointerMotionMask 
        | ButtonPressMask 
        | ButtonReleaseMask);

    Atom WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", false);
    XSetWMProtocols(display, window, &WM_DELETE_WINDOW, 1);

    Atom _NET_WM_WINDOW_TYPE = XInternAtom(display, "_NET_WM_WINDOW_TYPE", false);
    Atom type = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", false);
    XChangeProperty(display, window,
        _NET_WM_WINDOW_TYPE, 
        XA_ATOM, 
        32,
        PropModeReplace, 
        (unsigned char*) &type, 
        1
    );

    /* Create a graphics context for the window */
    GC gc = XCreateGC(display, window, 0, 0);
    XSetForeground(display, gc, black);
    XSetBackground(display, gc, white);

    /* Split the text down into a list of lines */
    char ** lines = nullptr;
    static const size_t textLen = strlen(text) + 1;
    size_t numLines{};
    char * textCopy = new char[textLen];
    strncpy(textCopy, text, textLen);

    char const * linePtr = strtok(textCopy, "\n");
    while (linePtr != nullptr)
    {
        char ** newLines = new char*[numLines + 1];
        for (size_t i = 0; i < numLines; ++i)
            newLines[i] = lines[i];
        delete[] lines;
        lines = newLines;
        lines[numLines] = new char[strlen(linePtr) + 1];
        strncpy(lines[numLines], linePtr, strlen(linePtr) + 1);
        ++numLines;
        linePtr = strtok(nullptr, "\n");
    }
    delete[] textCopy;

    /* Compute the printed length and height of the longest and the tallest line */
    XFontStruct * font(XQueryFont(display, XGContextFromGC(gc)));
    if(!font) 
        return;

    int length{}, height{}, direction{}, ascent{}, descent{};
    XCharStruct overall{};
    for (size_t i = 0; i < numLines; ++i)
    {
        XTextExtents(font,
            lines[i], 
            static_cast<int>(strlen(lines[i])),
            &direction, 
            &ascent, 
            &descent, 
            &overall
        );
        length = overall.width > length ? overall.width : length;
        height = (ascent + descent) > height ? (ascent+descent) : height;
    }

    /* Compute the shape of the window, needed to display the text and adjust the window accordingly */
    const int X = DisplayWidth(display, DefaultScreen(display)) / 2 - length / 2 - 10;
    const int Y = DisplayHeight(display, DefaultScreen(display)) / 2 - static_cast<int>(height / 2 - height - 10);
    const int W = length + 20;
    const int H = static_cast<int>(numLines * height + height + 40);
    XMoveResizeWindow(display, window, X, Y, W, H);

    /* Compute the shape of the OK button */
    XTextExtents(font, "OK", 2, &direction, &ascent, &descent, &overall);
    const int okWidth = overall.width;
    const int okHeight = ascent + descent;
    const int okX1 = W / 2 - okWidth / 2 - 15;
    const int okY1 = static_cast<int>(numLines * height + 20) + 5;
    const int okX2 = W / 2 + okWidth / 2 + 15;
    const int okY2 = okY1 + 4 + okHeight;
    const int okBaseX = okX1 + 15;
    const int okBaseY = okY1 + 2 + okHeight;

    //XFreeFontInfo(nullptr, font, 1); /* We don't need that anymore */

    /* Make the window non resizeable */
    XSizeHints hints{};
    hints.flags = PSize | PMinSize | PMaxSize;
    hints.min_width = hints.max_width = hints.base_width = W;
    hints.min_height = hints.max_height = hints.base_height = H;
    XSetWMNormalHints(display, window, &hints);

    XMapRaised(display, window);
    XFlush(display);

    /* Event loop */
    bool run = true, buttonFocus = false;
    do 
    {
        XEvent event{};
        XNextEvent(display, &event);
        int offset{};

        if (event.type == MotionNotify) 
        {
            if (event.xmotion.x >= okX1 && 
                event.xmotion.x <= okX2 && 
                event.xmotion.y >= okY1 && 
                event.xmotion.y <= okY2) 
            {
                if (!buttonFocus) 
                    event.type = Expose;
                buttonFocus = true;
            } 
            else 
            {
                if (buttonFocus) 
                    event.type = Expose;
                buttonFocus = false;
                offset = 0;
            }
        }

        switch(event.type) 
        {
        case ButtonPress:
        case ButtonRelease:
            if (event.xbutton.button != Button1) 
                break;

            if (buttonFocus) 
            {
                offset = event.type == ButtonPress ? 1 : 0;
                if (!offset) 
                    run = false;
            } 
            else 
            {
                offset = 0;
            }
            break;

        case Expose:
        case MapNotify:
            XClearWindow(display, window);

            /* Draw text lines */
            for (size_t i = 0; i < numLines; ++i)
            {
                XDrawString(display, 
                    window, 
                    gc, 
                    10, 
                    static_cast<int>(10 + height + height * i), 
                    lines[i], 
                    static_cast<int>(strlen(lines[i]))
                );
            }

            /* Draw OK button */
            if (buttonFocus) 
            {
                XFillRectangle(display, window, gc, offset + okX1, offset + okY1, okX2 - okX1, okY2 - okY1);
                XSetForeground(display, gc, white);
            } 
            else 
            {
                XDrawLine(display, window, gc, okX1, okY1, okX2, okY1);
                XDrawLine(display, window, gc, okX1, okY2, okX2, okY2);
                XDrawLine(display, window, gc, okX1, okY1, okX1, okY2);
                XDrawLine(display, window, gc, okX2, okY1, okX2, okY2);
            }

            XDrawString(display, window, gc, offset + okBaseX, offset+okBaseY, "OK", 2);

            if (buttonFocus)
                XSetForeground(display, gc, black);

            XFlush(display);
            break;

        case KeyRelease:
            if (XLookupKeysym(&event.xkey, 0) == XK_Escape) 
                run = false;
            break;

        case ClientMessage:
            if(event.xclient.data.l[0] == WM_DELETE_WINDOW)
                run = false;
            break;
        };
    } while (run);

    for (size_t i = 0; i < numLines; ++i)
        delete[] lines[i];
    delete[] lines;

    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
}
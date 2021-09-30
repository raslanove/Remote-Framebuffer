/* 
 * File:   main.c
 * Author: raslanove
 *
 * Created on September 29, 2021
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <stdio.h>
#include <stdlib.h>

static struct {
    Display *display;
    int screen;
    Window window;
    GC graphicsContext;
} X11;

static const char *EVENT_NAME[] = {"Reserved", "Reserved", "KeyPress", "KeyRelease", "ButtonPress", "ButtonRelease", "MotionNotify", "EnterNotify", "LeaveNotify", "FocusIn", "FocusOut", "KeymapNotify", "Expose", "GraphicsExpose", "NoExpose", "VisibilityNotify", "CreateNotify", "DestroyNotify", "UnmapNotify", "MapNotify", "MapRequest", "ReparentNotify", "ConfigureNotify", "ConfigureRequest", "GravityNotify", "ResizeRequest", "CirculateNotify", "CirculateRequest", "PropertyNotify", "SelectionClear", "SelectionRequest", "SelectionNotify", "ColormapNotify", "ClientMessage", "MappingNotify", "GenericEvent", "LASTEvent"};

static void startX11() {

    // Connect to X11,
    X11.display = XOpenDisplay(0);
    X11.screen = DefaultScreen(X11.display);
    
    // Create window,
    X11.window = XCreateSimpleWindow(
            X11.display, XDefaultRootWindow(X11.display), 
            0, 0, 300, 300, 
            0 /*border width*/, XBlackPixel(X11.display, X11.screen) /*border*/,
            0xffffffff /* background*/);
    XSetStandardProperties(
            X11.display, X11.window, 
            "besm Allah" /*window name*/, "besm Allah" /*icon name*/, None /*icon*/,
            NULL, 0, NULL); // Extra arguments.
    
    // Create graphics context,
    X11.graphicsContext = XCreateGC(X11.display, X11.window, 0, 0);    
    
    // Show the window,
    XMapRaised(X11.display, X11.window);
    
    // Register input events,
    XSelectInput(X11.display, X11.window, ExposureMask | ButtonPressMask | KeyPressMask);
}

static void closeX11() {
    XFreeGC(X11.display, X11.graphicsContext);
    XDestroyWindow(X11.display, X11.window);
    XCloseDisplay(X11.display);    
}

static void draw() {
    XSetBackground(X11.display, X11.graphicsContext, 0xffffffff);
    XClearWindow(X11.display, X11.window);
}

int main() {

    XEvent event;
    KeySym key;
    int mouseLastX=0, mouseLastY=0;
    char text[256];

    printf("besm Allah :)\n");
    
    startX11();
    while (1) {
        
        // Blocking function call...
        XNextEvent(X11.display, &event);
       
        printf("Received event: %s\n", EVENT_NAME[event.type]);
        
        // Expose is received on resize (not sure if this is the only case),
        if (event.type==Expose && event.xexpose.count==0) draw();
        
        // Handle key presses,
        if (event.type==KeyPress && XLookupString(&event.xkey, text, 255, &key, 0) == 1) {
            if (text[0]=='q') {
                closeX11();
                break;
            }
        }
        
        // Handle mouse downs,
        if (event.type==ButtonPress) {
            
            // Draw line,
            XSetForeground(X11.display, X11.graphicsContext, 0xffff0000);
            XDrawLine(X11.display, X11.window, X11.graphicsContext, mouseLastX, mouseLastY, event.xbutton.x, event.xbutton.y);
            mouseLastX = event.xbutton.x;
            mouseLastY = event.xbutton.y;
            
            // Draw text,
            XSetForeground(X11.display, X11.graphicsContext, 0xff0000ff);
            strcpy(text, "Hello!");
            XDrawString(X11.display, X11.window, X11.graphicsContext, event.xbutton.x, event.xbutton.y, text, strlen(text));
        }
    }
    
    return 0;
}
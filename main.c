/* 
 * File:   main.c
 * Author: raslanove
 *
 * Created on September 29, 2021
 */

// References: https://www.x.org/releases/X11R7.6/doc/libX11/specs/libX11/libX11.html#Transferring_Images_between_Client_and_Server

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>


#define WINDOW_WIDTH 300
#define WINDOW_HEIGHT 300

static struct {
    Display *display;
    int screen;
    Window window;
    GC graphicsContext;
    XImage backBufferImage;
    int32_t backBufferImageData[WINDOW_WIDTH*WINDOW_HEIGHT];
} X11;

static const char *EVENT_NAME[] = {"Reserved", "Reserved", "KeyPress", "KeyRelease", "ButtonPress", "ButtonRelease", "MotionNotify", "EnterNotify", "LeaveNotify", "FocusIn", "FocusOut", "KeymapNotify", "Expose", "GraphicsExpose", "NoExpose", "VisibilityNotify", "CreateNotify", "DestroyNotify", "UnmapNotify", "MapNotify", "MapRequest", "ReparentNotify", "ConfigureNotify", "ConfigureRequest", "GravityNotify", "ResizeRequest", "CirculateNotify", "CirculateRequest", "PropertyNotify", "SelectionClear", "SelectionRequest", "SelectionNotify", "ColormapNotify", "ClientMessage", "MappingNotify", "GenericEvent", "LASTEvent"};

static void startX11() {

    
    // Connect to X11,
    X11.display = XOpenDisplay(0);
    X11.screen = DefaultScreen(X11.display);
    
    // Create window,
    X11.window = XCreateSimpleWindow(
            X11.display, XDefaultRootWindow(X11.display), 
            0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 
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
    
    // Create a back-buffer image,
    memset(&X11.backBufferImage, 0, sizeof(X11.backBufferImage));
    X11.backBufferImage.width  = WINDOW_WIDTH ;
    X11.backBufferImage.height = WINDOW_HEIGHT;
    X11.backBufferImage.xoffset = 0;
    X11.backBufferImage.format = ZPixmap;
    X11.backBufferImage.data = (char *) X11.backBufferImageData;
    X11.backBufferImage.byte_order = LSBFirst;
    X11.backBufferImage.bitmap_unit = 8;        // Row bit alignment. See: https://www.x.org/releases/X11R7.5/doc/man/man3/XPutPixel.3.html
    X11.backBufferImage.bitmap_bit_order = LSBFirst;
    X11.backBufferImage.bitmap_pad = 8;
    X11.backBufferImage.depth = 24;             // Must be the same as the target drawable (window).
    X11.backBufferImage.bytes_per_line = 0;
    X11.backBufferImage.bits_per_pixel = 32;
    X11.backBufferImage.red_mask   = 0x00ff0000;
    X11.backBufferImage.green_mask = 0x0000ff00;
    X11.backBufferImage.blue_mask  = 0x000000ff;
    X11.backBufferImage.obdata = 0;          // This (char *) shall be passed to events?
    
    if (!XInitImage(&X11.backBufferImage)) printf("Image creation failed!\n");
}

static void closeX11() {
    XFreeGC(X11.display, X11.graphicsContext);
    XDestroyWindow(X11.display, X11.window);
    XCloseDisplay(X11.display);    
}

static void clear() {
    XSetBackground(X11.display, X11.graphicsContext, 0xffffffff);
    XClearWindow(X11.display, X11.window);
}

static int32_t drawColor=0x00f0ff00;
static void draw() {

    // Fill the back-buffer with an arbitrary color,
    drawColor+=30;
    int index=0;
    for (int y=0; y<X11.backBufferImage.height; y++) {
        for (int x=0; x<X11.backBufferImage.width; x++) {
            X11.backBufferImageData[index++] = drawColor;
        }
    }
    
    // Draw image,
    XPutImage(X11.display, X11.window, X11.graphicsContext, &X11.backBufferImage, 0, 0, 0, 0, X11.backBufferImage.width, X11.backBufferImage.height);
}

int main() {

    XEvent event;
    KeySym key;
    int mouseLastX=0, mouseLastY=0;
    char text[256];

    printf("besm Allah :)\n");
    
    startX11();
    while (1) {

        // If you need to do something while waiting for events,
        if (!XPending(X11.display)) {
            usleep(10000);
            continue;
        }
        
        // Blocking function call...
        XNextEvent(X11.display, &event);
       
        printf("Received event: %s\n", EVENT_NAME[event.type]);
        
        // Expose is received on resize (not sure if this is the only case),
        if (event.type==Expose && event.xexpose.count==0) clear();
        
        // Handle key presses,
        if (event.type==KeyPress && XLookupString(&event.xkey, text, 255, &key, 0) == 1) {
            if (text[0]=='q') {
                closeX11();
                break;
            }
        }
        
        // Handle mouse downs,
        if (event.type==ButtonPress) {
            
            draw();
            
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

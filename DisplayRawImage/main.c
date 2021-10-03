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
#include <poll.h>
#include <time.h>

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

#define XVFB_OVERHEAD_BYTES 3232

static struct {
    Display *display;
    int screen;
    Window window;
    GC graphicsContext;
    XImage backBufferImage;
    int32_t backBufferImageData[(XVFB_OVERHEAD_BYTES/4) + (WINDOW_WIDTH*WINDOW_HEIGHT)];
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
    // See: https://chromium.googlesource.com/chromium/src.git/+/62.0.3178.1/content/browser/compositor/software_output_device_x11.cc
    memset(&X11.backBufferImage, 0, sizeof(X11.backBufferImage));
    X11.backBufferImage.width  = WINDOW_WIDTH ;
    X11.backBufferImage.height = WINDOW_HEIGHT;
    X11.backBufferImage.xoffset = 0;
    X11.backBufferImage.format = ZPixmap;
    X11.backBufferImage.data = (char *) &X11.backBufferImageData[XVFB_OVERHEAD_BYTES/4];
    X11.backBufferImage.byte_order = LSBFirst;
    X11.backBufferImage.bitmap_unit = 8;        // Row bit alignment. See: https://www.x.org/releases/X11R7.5/doc/man/man3/XPutPixel.3.html
    X11.backBufferImage.bitmap_bit_order = LSBFirst;
    X11.backBufferImage.bitmap_pad = 8;
    X11.backBufferImage.depth = DefaultDepthOfScreen(ScreenOfDisplay(X11.display, X11.screen));     // Must be the same as the target drawable (window).
    X11.backBufferImage.bytes_per_line = 0;
    X11.backBufferImage.bits_per_pixel = 32;
    X11.backBufferImage.red_mask   = 0x00ff0000;
    X11.backBufferImage.green_mask = 0x0000ff00;
    X11.backBufferImage.blue_mask  = 0x000000ff;
    X11.backBufferImage.obdata = 0;          // This (char *) shall be passed to events?
    
    if (!XInitImage(&X11.backBufferImage)) printf("Image creation failed!\n");
}

static void closeX11() {
    // To use the destroy image routine, both the image structure and data must be
    // dynamically allocated, since they'll be freed,
    //XDestroyImage(&X11.backBufferImage);
    XFreeGC(X11.display, X11.graphicsContext);
    XDestroyWindow(X11.display, X11.window);
    XCloseDisplay(X11.display);    
}

static void clear() {
    XSetBackground(X11.display, X11.graphicsContext, 0xffffffff);
    XClearWindow(X11.display, X11.window);
}

static void someX11Functions() {
    /*
    // Draw line,
    XSetForeground(X11.display, X11.graphicsContext, 0xffff0000);
    XDrawLine(X11.display, X11.window, X11.graphicsContext, x1, y1, x2, y2);

    // Draw text,
    if (event.type==ButtonPress) {
        XSetForeground(X11.display, X11.graphicsContext, 0xff0000ff);
        strcpy(text, "Hello!");
        XDrawString(X11.display, X11.window, X11.graphicsContext, event.xbutton.x, event.xbutton.y, text, strlen(text));
    }
    */
}

static void draw() {
    // Draw image,
    XPutImage(X11.display, X11.window, X11.graphicsContext, &X11.backBufferImage, 0, 0, 0, 0, X11.backBufferImage.width, X11.backBufferImage.height);
}

static double timeMillisSince(struct timespec* startTime) {    
    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);
    double timeNanos = 
        (currentTime.tv_sec  - startTime->tv_sec )*1E9 +
        (currentTime.tv_nsec - startTime->tv_nsec);
    return timeNanos / 1E6;
}

int main() {

    struct pollfd pollFileDescriptor; 
    int32_t imageBytesReceived=0;
    int32_t imageTargetBytesCount = (WINDOW_WIDTH*WINDOW_HEIGHT*4) + XVFB_OVERHEAD_BYTES;
    
    XEvent event;
    KeySym key;
    char text[256];

    printf("besm Allah :)\n");
    
    // Initialize polling structure,
    // See: https://linuxhint.com/use-poll-system-call-c/
    // and: https://linux.die.net/man/2/poll
    // and: https://pubs.opengroup.org/onlinepubs/009696799/functions/poll.html
    pollFileDescriptor.fd = 0;
    pollFileDescriptor.events = POLLIN;
    
    startX11();
    int32_t exposed=0;
    int32_t sleepPeriod=1;
    struct timespec startTime;
    while (1) {

        int32_t readBytesCount=0;
        if (exposed) {
            // Check if something is ready on the input stream,            
            poll(&pollFileDescriptor, 1 /*one descriptor provided*/, 0 /*return immediately without timeout*/);
            if (pollFileDescriptor.revents & POLLIN) {

                // We've received some data,
                readBytesCount = read(0 /*stdin*/, &((char *) X11.backBufferImageData)[imageBytesReceived], imageTargetBytesCount-imageBytesReceived);                
                imageBytesReceived += readBytesCount;
                if (imageBytesReceived == imageTargetBytesCount) {
                    draw();
                    imageBytesReceived = 0;
                    printf("Reading took: %f\n", timeMillisSince(&startTime));
                }
                
                // Expecting more input, so don't sleep too much,
                sleepPeriod = 1;
            }
        }
        
        // If no events to handle, sleep,
        if (!XPending(X11.display)) {
            
            // Sleep only if no input is pending,
            if (!readBytesCount) {
                usleep(sleepPeriod);
                
                // Sleep more next time, with a cap,
                sleepPeriod *= 2;
                if (sleepPeriod>10000) sleepPeriod = 10000;
            }
            continue;
        }
        
        // Blocking function call,
        XNextEvent(X11.display, &event);
        printf("Received event: %s\n", EVENT_NAME[event.type]);
        
        // Expose is received on resize (not sure if this is the only case),
        if (event.type==Expose && event.xexpose.count==0) {
            exposed = 1;
            clock_gettime(CLOCK_REALTIME, &startTime);
        }
        
        // Handle key presses,
        if (event.type==KeyPress && XLookupString(&event.xkey, text, 255, &key, 0) == 1) {
            if (text[0]=='q') {
                closeX11();
                break;
            }
        }
    }
    
    return 0;
}

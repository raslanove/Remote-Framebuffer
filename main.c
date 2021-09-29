#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <stdio.h>
#include <stdlib.h>

Display *display;
int screen;
Window window;
GC graphicsContext;

void startX11() {
    display = XOpenDisplay(0);
    screen = DefaultScreen(display);
    
    window = XCreateSimpleWindow(display, XDefaultRootWindow(display), 0, 0, 300, 300, 5, 0xffffffff, 0x00000000);
    XSetStandardProperties(display, window, "besm Allah", "besm Allah AlRa7maan AlRa7eem", None, NULL, 0, NULL);
    XSelectInput(display, window, ExposureMask | ButtonPressMask | KeyPressMask);
    graphicsContext = XCreateGC(display, window, 0, 0);    
    
    XSetBackground(display, graphicsContext, 0xffffffff);
    XSetForeground(display, graphicsContext, 0x00000000);
    XClearWindow(display, window);
    
    XMapRaised(display, window);
}

void closeX11() {
    XFreeGC(display, graphicsContext);
    XDestroyWindow(display, window);
    XCloseDisplay(display);    
}

void draw() {
    XClearWindow(display, window);
}

int main() {

    XEvent event;
    KeySym key;
    char text[256];

    printf("besm Allah :)\n");
    
    startX11();
    while (1) {
        XNextEvent(display, &event);
        if (event.type==Expose && event.xexpose.count==0) {
            draw();
        }
        if (event.type==KeyPress && XLookupString(&event.xkey, text, 255, &key, 0) == 1) {
            if (text[0]=='q') {
                closeX11();
                break;
            }
            printf("You pressed the %c key!\n", text[0]);
        }
        if (event.type==ButtonPress) {
            int x = event.xbutton.x, y=event.xbutton.y;
            XSetForeground(display, graphicsContext, 0xff000000);
            XDrawLine(display, window, graphicsContext, x, y, x, y);
            XSetForeground(display, graphicsContext, 0xff0000ff);
            strcpy(text, "Hello!");
            XDrawString(display, window, graphicsContext, x, y, text, strlen(text));            
        }
    }
    
    return 0;
}

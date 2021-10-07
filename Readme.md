# Remote Framebuffer
I have a machine running Ubuntu 20.04 with two monitors attached to a single display adapter. I attached two keyboards and two mice, then started to look for solutions on how to effectively allow two users to use this machine at the same time.

We have mutiple options:
- ### Virtualbox
	One user would work on the host operating system and the other inside the virtual machine. Virtualbox allows you to attach usb devices from the host machine to the guest guest machine, has basic graphics acceleration support (OpenGl 2.1 if I remember correctly, which is pretty useless) and sound works out of the box (you can add a cheap USB sound adapter to the mix and dedicate it to the virtual machine). But, it takes a slice from your system resources (cpu and memory) and dedicates it to the guest machine. These limits the resources available to both the host and the guest machines. Even if one of them is idle, the other won't be able to benefit from these resources.

- ### Proxmox VE
	If your machine supports **Intel vt-d** or **AMD-V** and has multiple graphics adapters, you can use a hypervisor solution like Proxmox VE to split your machine resources among two virtual machines. This way the two machines can have real hardware acceleration and you don't have to deal with multiple sets of drivers in the same system. Unfortunately, I have only one graphics adapter and I don't want to buy another one.

- ### Docker
	If you are patient enough, you can have containers with hardware acceleration, and can attach USB devices to the container. But from my experience, there's no generic solution, as it depends on the specific hardware in your machine. I honestly couldn't get it to work on my system. Projects like [X11Docker](https://github.com/mviereck/x11docker) are supposed to make things easier, but I didn't try. 
I have also come across [this](https://www.collabora.com/news-and-blog/blog/2019/04/01/running-android-next-to-wayland/), but didn't investigate it long enough.

- ### Xvfb
	Xvfb is an **X11 server** that just works! It provides virtual frame-buffers to replace the need for actual display adapters. These virtual displays can also have their separate input mechanisms. You can run as many instances as you need, and they don't interfere with your system's actual X11/Wayland server. Thankfully, on Ubuntu 20.04 we have mature **Mesa** drivers that have **llvmpipe** based software rendering supporting OpenGL 4.5, so these virtual displays will still be able to run graphics intensive applications/games, albeit on the CPU not the GPU. 

	The problem is, they are virtual! You need a way to transfer the contents of the virtual frame-buffer to the real graphics adapter, so you can have your separate X11 systems, and a way to dedicate a mouse and a keyboard to each one. Most people would use VNC to view the contents of the frame-buffer, but I found it not suitable for the high-framerates I desired. I ended up making this simple solution that we have here.
	
## Technique
1. ### Create a virtual frame-buffer that is memory-mapped to a file
	
		export DISPLAY=:2
		mkdir ~/Desktop/Xvfb
		Xvfb $DISPLAY -screen 0 1600x900x24 -fbdir ~/Desktop/Xvfb &

	In Ubuntu 20.04, displays 0 and 1 are typically taken by the actual X11 server. So, we pick an unused display id. Any number > 1 should be fine.
	The trick here is, X11 applications send they X11 commands to the display specified in the DISPLAY environment variable. So, any X11 apps launched after setting this variable should automatically run on the virtual display. If you want, you can set this environment variable per app, like this:

		DISPLAY=:2 blender
	This should run blender on our virtual display, even if the DISPLAY environment variable wasn't set globally to it.
	Back to our command. Memory mapping the frame-buffer to a file allows us to capture its contents whenever we need, and indeed this is what we are going to do.

2. ### Periodically capture and display frame-buffer contents
	Build our nice apps as following:
	
		sudo apt-get install libx11-dev
		gcc DisplayRawImage/display.c -O3 -lX11 -o display.o
		gcc FrameBufferCapture/capture.c -O3 -o capture.o

	Now we can use them to capture and display the captured frame-buffer:

		./capture.o ~/Desktop/Xvfb/Xvfb_screen0 60 | DISPLAY=:0 ./display.o &

	This reads our memory mapped virtual frame-buffer file at a rate of 60 frames per seconds and pipes it to our display utility to display it on a window on the actual display (:0). You can proceed now to run your almost entirely separate desktop on this display:
	
		sudo apt-get install lxde
		startlxde &

3.  ### Capture and forward USB keyboard and mouse
	Use our other project, [RemoteHID](https://github.com/raslanove/RemoteHID) for this.

## Issues

We don't have control on **v-sync**. Fast moving animations can have slight artifacts. X11 seems to have no notion of vertical synchronization. You'll have to write a new display utility, that either uses **OpenGL** and does the buffer swapping thing, or uses **[libdrm](https://github.com/dvdhrm/docs/blob/master/drm-howto/modeset-vsync.c)**. 


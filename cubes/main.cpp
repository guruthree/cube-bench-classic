// a lot of safety checks are _not_ being done

#include <QDOffscreen.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "vectormath2.h"
#include "bubblesort.h"
#include "cube.h"

// how much smaller than the resolution to make the window
#define SHRINK 40
// a smaller resolution is needed for the emulator
//#define SHRINK 160

// use back buffer to render to, at the cost of some performance
#define USEOFFSCREEN

// clear the entire/canvase back buffer 
// (instead of just the cube by writing over it)
#define ERASECANVAS

void InitToolbox()
{
	InitGraf((Ptr)&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	FlushEvents(everyEvent, 0);
	TEInit();
	InitDialogs(0L);
	InitCursor();
}

// display the Ticks Per Frame (1 tick ~= 1/60 s)
void writeTPF(char buffer[], unsigned char TPF)
{
	sprintf(buffer, "TPF: %hu", TPF);
	CtoPstr(buffer);
	MoveTo(7, 15);
	DrawString((unsigned char *)buffer);
}

void main()
{
	// for looping
	int i;

	// window & input variables
	int xRes, yRes;
	WindowPtr appWindow;
	Rect windowRect;
	Boolean running;
	EventRecord myEvent;
	char keyChar;

	// graphics variables
#ifdef USEOFFSCREEN
	GDHandle onscreenDevice;
	GWorldPtr onScreen, offScreen;
	PixMapHandle pixels;
	RgnHandle updateRgn, tpfRgn, fpuRgn;
#endif

	// cube variables
	Cube myCube(100);
	Boolean wireFrame = true;

	// benchmark variables
	unsigned char TPF = 0; // ticks per frame
	char buffer[20]; // for displaying TPF
	unsigned long int last; // time of last frame

	// initialisation
	InitToolbox();
	char FPUbuffer[20] = "";
	long FPUtype;
	if (noErr == Gestalt(gestaltFPUType, &FPUtype))
	{
		switch (FPUtype)
		{
			case gestaltNoFPU:
				strcpy(FPUbuffer, "NO FPU");
				break;
				
			case gestalt68881:
				strcpy(FPUbuffer, "FPU 68881");
				break;
				
			case gestalt68882:
				strcpy(FPUbuffer, "FPU 68882");
				break;
			
			case gestalt68040FPU:
				strcpy(FPUbuffer, "FPU 68040");
				break;
				
			default:
				strcpy(FPUbuffer, "FPU ERR");
				break;
		}
	}
	else
	{
		strcpy(FPUbuffer, "FPU ERR");
	}
#if !mc68881
	if (FPUbuffer[4] == '6')
		strcat(FPUbuffer, " (DISABLED)");
#endif
	CtoPstr(FPUbuffer);
	

	// get current monitor resolution
	windowRect = (*(GetGDevice()))->gdRect;
	// convert to window size by shrinking
	windowRect.top += SHRINK + 32;
	windowRect.bottom -= SHRINK;
	windowRect.left += SHRINK;
	windowRect.right -= SHRINK;
	// calculate the window resolution
	xRes = windowRect.right - windowRect.left;
	yRes = windowRect.bottom - windowRect.top;

	// create window, sized by windowRect in absolute screen coordinates
	appWindow = NewWindow(0L, &windowRect, "\pSpinning Cube Benchmark",
		true, movableDBoxProc, (WindowPtr)-1L, 1, 0);
		// 0 - auto assign memory, window size, window title, visible
		// window type, put window in front, have close box, refCon
	ShowWindow(appWindow); // bring to front
	SetPort(appWindow);	   // make the window the quickdraw graphics target
	
	// coloring stuff
	long fgColor = blackColor;
	long bgColor = whiteColor;
	Boolean invertColor = false;

#ifdef USEOFFSCREEN
	// front buffer
	GetGWorld(&onScreen, &onscreenDevice);
	// create back buffer (pixel depth 0 reuses screen depth)
	NewGWorld(&offScreen, 0, &(appWindow->portRect), NULL, NULL, 0);
	// "ctSeed slamming" to avoid having to remap colors on blit
	(*( (*( offScreen->portPixMap ) )->pmTable) )->ctSeed = 
		(*( (*( (*(GetGDevice()))->gdPMap) )->pmTable) )->ctSeed;
	// get memory of back buffer & lock to keep from being destroyed
	pixels = GetGWorldPixMap(offScreen);
	LockPixels(pixels);
	
	// screen area where TPF is drawn
	tpfRgn = NewRgn();
	SetRectRgn(tpfRgn, 0, 0, 80, 40);
	// screen area where FPU is drawn 
	updateRgn = NewRgn();
	SetRectRgn(updateRgn, 0, yRes-40, 80, yRes);
	// combine in tpfRgn
	UnionRgn(tpfRgn, updateRgn, tpfRgn);

	// make sure the back buffer is clear
	SetGWorld(offScreen, NULL);
	EraseRect(&appWindow->portRect);
	SetGWorld(onScreen, onscreenDevice);
#endif

	running = true;
	while (running == true)
	{
		last = TickCount();
	
		if (WaitNextEvent(everyEvent, &myEvent, 1, NULL))
		{
			switch (myEvent.what)
			{
				case activateEvt:
					// quit when we loose focus
					if ((myEvent.modifiers & activeFlag) == 0)
					{
						running = false;
					}
					break;

				case keyDown:
					keyChar = myEvent.message & charCodeMask;
					switch (keyChar)
					{
						// quit
						case 'q':
						case 'Q':
						case 0x1b: // escape
							running = false;
							break;

						// uiojkl to control rotation
						case 'j':
							xangle -= 0.1;
							break;
						case 'l':
							xangle += 0.1;
							break;
						case 'i':
							yangle -= 0.1;
							break;
						case 'k':
							yangle += 0.1;
							break;
						case 'u':
							zangle -= 0.1;
							break;
						case 'o':
							zangle += 0.1;
							break;
						
						// UIOJKL to control rotation speed
						case 'J':
							dxangle -= 0.01;
							break;
						case 'L':
							dxangle += 0.01;
							break;
						case 'I':
							dyangle -= 0.01;
							break;
						case 'K':
							dyangle += 0.01;
							break;
						case 'U':
							dzangle -= 0.01;
							break;
						case 'O':
							dzangle += 0.01;
							break;
							
						// stop auto rotation
						case 's':
						case 'S':
						case ' ':
							if (dxangle == 0 && dyangle == 0 && dzangle == 0) {
								dxangle = olddxangle;
								dyangle = olddyangle;
								dzangle = olddzangle;
							}
							else {
								olddxangle = dxangle;
								olddyangle = dyangle;
								olddzangle = dzangle;
								dxangle = 0;
								dyangle = 0;
								dzangle = 0;
							}
							break;

						// randomize rotation & speed
						case 'r':
						case 'R':
							// TODO
							break;
						
						// black background, white text
						case 'b':
							invertColor = !invertColor;
							last = bgColor;
							bgColor = fgColor;
							fgColor = last;
#ifdef USEOFFSCREEN
		SetGWorld(offScreen, NULL);
#endif
							BackColor(bgColor);
							ForeColor(fgColor); 
#ifdef USEOFFSCREEN
		SetGWorld(onScreen, onscreenDevice);
#endif
//							break;
						
						// erase buffer
						case 'e':
						case 'E':
#ifdef USEOFFSCREEN
							SetGWorld(offScreen, NULL);
#endif
							EraseRect(&appWindow->portRect);
#ifdef USEOFFSCREEN
							SetGWorld(onScreen, onscreenDevice);
							CopyBits(&(((GrafPtr)offScreen)->portBits),
									 &(((GrafPtr)onScreen)->portBits),
									 &(offScreen->portRect),
									 &(onScreen->portRect),
									 srcCopy, NULL);
#endif
							break;

						// f filled sides
						case 'f':
						case 'F':
							wireFrame = !wireFrame;
						 	break;
						
						// increase cube size
						case '+':
							myCube.increaseSize();
							break;
						// decrease cube size
						case '-':
							myCube.decreaseSize();
							break;

						default:
						 	break;
					}
					break;

				default:
					break;
			}
		}

		// erase old cube, process cube, & render new cube

#ifdef USEOFFSCREEN
		// activate back buffer
		SetGWorld(offScreen, NULL);
#endif
		// erase the bound of the window on the current GWorld
#ifdef ERASECANVAS
		EraseRect(&appWindow->portRect);
#else // erase just the pixels of the cube
		// clear TPF/FPU text
		EraseRgn(tpfRgn);
		
		// draw over old cube
		if (wireFrame)
		{
			ForeColor(bgColor);
			myCube.draw(false);
		}
		else
		{
			// probably too expensive to draw precisely over the cube
			// draw a circle (or close square) over the rotation area isntead
			myCube.roughBounds(updateRgn);
			EraseRgn(updateRgn);
			// could be EraseOval (if we got a Rect instead of a Rgn)
			// or myCube.solidCibe(false);
		}
#endif

		// rotate to new position
		myCube.rotate();
		// pre-calculate for rendering
		myCube.preCalculate(xRes, yRes);

		if (wireFrame)
			myCube.draw(true);
		else
			myCube.solidCube(true);

		// write TPF/FPU
		ForeColor(fgColor);
		writeTPF(buffer, TPF);
		MoveTo(7, yRes - 7);
		DrawString((unsigned char *)FPUbuffer);

		// copy back buffer to front, which will also trigger a display refresh
#ifdef USEOFFSCREEN
		// only update where the cube is
//		if (wireFrame)
//			myCube.bounds(rotatedCube, updateRgn);
		UnionRgn(updateRgn, tpfRgn, updateRgn);

		SetGWorld(onScreen, onscreenDevice);
		CopyBits(&(((GrafPtr)offScreen)->portBits),
				 &(((GrafPtr)onScreen)->portBits),
				 &(offScreen->portRect),
				 &(onScreen->portRect),
//				 srcCopy, updateRgn); // copy just upated region
				 srcCopy, NULL); // copy entire screen
#endif
		TPF = TickCount() - last;

	} // while running

#ifdef USEOFFSCREEN
	DisposeRgn(tpfRgn);
	DisposeRgn(updateRgn);
	UnlockPixels(pixels);
	DisposeGWorld(offScreen);
#endif
	DisposeWindow(appWindow);
}

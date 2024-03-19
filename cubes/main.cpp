// a lot of safety checks are _not_ being done

#include <QDOffscreen.h>
#include <stdio.h>
#include <string.h>

#include "cube.h"
//#include "vectormath2.h"

// how much smaller than the resolution to make the window
//#define SHRINK 40
// a smaller resolution is needed for the emulator
#define SHRINK 160

// use back buffer to render to, at the cost of some performance
#define USEOFFSCREEN

// clear the entire/canvase back buffer 
// (instead of just the cube by writing over it)
#define ERASECANVAS

// how many cubes we could be rendering
#define NUM_CUBES 10

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
//	Cube cube = myCube(100);
	Cube *cubes[NUM_CUBES] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	cubes[0] = new Cube(100);
//	cubes[1] = new Cube(50);
	// default cube for control
	Cube *myCube = cubes[0];
	Boolean activeCubes[NUM_CUBES] = {true, false, false, false, false, false, false, false, false, false};

	Boolean doRotate = true;
	Boolean doMove = false;
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
						case '\r': // carraige return (enter)
							running = false;
							break;

						// uiojkl to control rotation
						case 'j':
							myCube->rotate(Vector3(-0.1,  0.0,  0.0));
							break;
						case 'l':
							myCube->rotate(Vector3( 0.1,  0.0,  0.0));
							break;
						case 'i':
							myCube->rotate(Vector3( 0.0, -0.1,  0.0));
							break;
						case 'k':
							myCube->rotate(Vector3( 0.0, +0.1,  0.0));
							break;
						case 'u':
							myCube->rotate(Vector3( 0.0,  0.0, -0.1));
							break;
						case 'o':
							myCube->rotate(Vector3( 0.0,  0.0, +0.1));
							break;
						
						// UIOJKL to control rotation speed
						case 'J':
							myCube->dangle.x -= 0.005f;
							break;
						case 'L':
							myCube->dangle.x += 0.005f;
							break;
						case 'I':
							myCube->dangle.y -= 0.005f;
							break;
						case 'K':
							myCube->dangle.y += 0.005f;
							break;
						case 'U':
							myCube->dangle.z -= 0.005f;
							break;
						case 'O':
							myCube->dangle.z += 0.005f;
							break;

						// tfghry to move on screen
						// TFGHRY to control movement speed
							
						// stop/start auto rotation
						case 's':
						case 'S':
						case ' ': // space
							doRotate = !doRotate;
							break;

						// stop/start movement
						case 'm':
						case 'M':
							doMove = !doMove;
							break;

						// randomize rotation & speed
						case 'r':
						case 'R':
							// TODO
							break;
						
						// black background, white text
						case 'b':
						case 'B':
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
							myCube->increaseSize();
							break;
						// decrease cube size
						case '-':
							myCube->decreaseSize();
							break;

						// activate a cube on keypress
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
						case '8':
						case '9':
						case '0':
							keyChar -= 48; // go from char to int
							if (keyChar == 0) // I want 0 to be 10
							{
								keyChar = 10;
							}
							keyChar--; // a '1' press should now be 0
							if (keyChar > NUM_CUBES)
								break; // safety
							if (cubes[keyChar] == NULL && !activeCubes[keyChar])
							{
								// cube is not currently active, make it so
								cubes[keyChar] = new Cube(20);
								activeCubes[keyChar] = true;
							}
							else
								activeCubes[keyChar] = !activeCubes[keyChar];
							if (activeCubes[keyChar])
								myCube = cubes[keyChar];
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
			myCube->draw(false);
		}
		else
		{
			// probably too expensive to draw precisely over the cube
			// draw a circle (or close square) over the rotation area isntead
			myCube->roughBounds(updateRgn, xRes, yRes);
			EraseRgn(updateRgn);
			// could be EraseOval (if we got a Rect instead of a Rgn)
			// or myCube->solidCibe(false);
		}
#endif

/*		// rotate to new position
		if (doRotate)
			myCube->autoRotate();
		// move to new position
		if (doMove)
			myCube->autoTranslate();
		// pre-calculate for rendering
		myCube->preCalculate(xRes, yRes);

		if (wireFrame)
			myCube->draw(true);
		else
			myCube->solidCube(true);*/

		for (i = 0; i < NUM_CUBES; i++)
		{
			if (activeCubes[i])
			{
				// rotate to new position
				if (doRotate)
					cubes[i]->autoRotate();
				// move to new position
				if (doMove)
				{
					cubes[i]->autoTranslate();
				}
				// pre-calculate for rendering
				cubes[i]->preCalculate(xRes, yRes);
				if (doMove)
				{				
					// reflection off the edges of the screen
					cubes[i]->calculateBounds();
					if (cubes[i]->leftBound <= 0 && cubes[i]->velocity.x < 0)
					{
						cubes[i]->velocity.x *= -1;
					}
					else if (cubes[i]->rightBound >= xRes && cubes[i]->velocity.x > 0)
					{
						cubes[i]->velocity.x *= -1;
					}
					
					if (cubes[i]->upperBound <= 0 && cubes[i]->velocity.y < 0)
					{
						cubes[i]->velocity.y *= -1;
					}
					else if (cubes[i]->lowerBound >= yRes && cubes[i]->velocity.y > 0)
					{
						cubes[i]->velocity.y *= -1;
					}
				}

				if (wireFrame)
					cubes[i]->draw(true);
				else
					cubes[i]->solidCube(true);
			}
		}

		// write TPF/FPU
		ForeColor(fgColor);
		writeTPF(buffer, TPF);
		MoveTo(7, yRes - 7);
		DrawString((unsigned char *)FPUbuffer);

		// copy back buffer to front, which will also trigger a display refresh
#ifdef USEOFFSCREEN
		// only update where the cube is
//		if (wireFrame)
//			myCube->bounds(rotatedCube, updateRgn);
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

	// get rid of any cubes we've created
	for (i = 0; i < NUM_CUBES; i++)
	{
		if (cubes[i] != NULL)
		{
			delete cubes[i];
		}
	}

#ifdef USEOFFSCREEN
	DisposeRgn(tpfRgn);
	DisposeRgn(updateRgn);
	UnlockPixels(pixels);
	DisposeGWorld(offScreen);
#endif
	DisposeWindow(appWindow);
}

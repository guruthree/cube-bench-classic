// a lot of safety checks are _not_ being done

#include <QDOffscreen.h>
#include <stdio.h>
#include <string.h>

#include "cube.h"
// #include "vectormath2.h"
#include "bubblesort.h"

// how much smaller than the resolution to make the window
// #define SHRINK 40
// a smaller resolution is needed for the emulator
#define SHRINK 160

// use back buffer to render to, at the cost of some performance
#define USEOFFSCREEN

// clear the entire/canvase back buffer
// (instead of just the cube by writing over it)
// #define ERASECANVAS

// how many cubes we could be rendering
#define NUM_CUBES 10

// boilerplate application initialisation
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
	RgnHandle updateRgn, tpfRgn, cubeRgn;
#endif

	// cube variables
	Cube *cubes[NUM_CUBES] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	Boolean activeCubes[NUM_CUBES] = {true, false, false, false, false, false, false, false, false, false};
	cubes[0] = new Cube(100);
	Cube *myCube = cubes[0]; // default cube for control
	int cubeAt = 0;
	float cubeDists[10];
	int cubeIdx[10];

	// cube display/interaction settings
	Boolean doRotate = true;
	Boolean doMove = false;
	Boolean wireFrame = true;
	Boolean invertColor = false;

	// benchmark variables
	unsigned char TPF = 0;	// ticks per frame
	char buffer[20];		// for displaying TPF
	unsigned long int last; // time of last frame
	// coloring stuff
	long fgColor = blackColor;
	long bgColor = whiteColor;

	// initialisation
	InitToolbox();

	// determine FPU info
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
	// a 6XXXX FPU
	if (FPUbuffer[4] == '6')
		strcat(FPUbuffer, " (DISABLED)");
#endif
	CtoPstr(FPUbuffer); // display functions expect P strings

	// get current monitor resolution to dermine window size
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

#ifdef USEOFFSCREEN
	// front buffer
	GetGWorld(&onScreen, &onscreenDevice);
	// create back buffer (pixel depth 0 reuses screen depth)
	NewGWorld(&offScreen, 0, &(appWindow->portRect), NULL, NULL, 0);
	// "ctSeed slamming" to avoid having to remap colors on blit
	(*((*(offScreen->portPixMap))->pmTable))->ctSeed =
		(*((*((*(GetGDevice()))->gdPMap))->pmTable))->ctSeed;
	// get memory of back buffer & lock to keep from being destroyed
	pixels = GetGWorldPixMap(offScreen);
	LockPixels(pixels);

	// screen area where TPF is drawn
	tpfRgn = NewRgn();
	SetRectRgn(tpfRgn, 0, 0, 80, 40);
	// screen area where FPU is drawn
	updateRgn = NewRgn();
	SetRectRgn(updateRgn, 0, yRes - 40, 80, yRes);
	// combine in tpfRgn
	UnionRgn(tpfRgn, updateRgn, tpfRgn);
	cubeRgn = NewRgn(); // screen space with cubes in it

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
					myCube->rotate(Vector3(-0.1, 0.0, 0.0));
					break;
				case 'l':
					myCube->rotate(Vector3(0.1, 0.0, 0.0));
					break;
				case 'i':
					myCube->rotate(Vector3(0.0, -0.1, 0.0));
					break;
				case 'k':
					myCube->rotate(Vector3(0.0, +0.1, 0.0));
					break;
				case 'u':
					myCube->rotate(Vector3(0.0, 0.0, -0.1));
					break;
				case 'o':
					myCube->rotate(Vector3(0.0, 0.0, +0.1));
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
					keyChar -= 48;	  // go from char to int
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

#ifdef ERASECANVAS
		// erase the bound of the window on the current GWorld
		EraseRect(&appWindow->portRect);
#else
		// erase just the pixels (or region?) of the cube
		// clear TPF/FPU text
		EraseRgn(tpfRgn);

		// draw over old cube
		for (i = 0; i < NUM_CUBES; i++)
		{
			if (activeCubes[i])
			{
				if (wireFrame)
				{
					// exact lines on top
					ForeColor(bgColor);
					cubes[i]->draw(false);
				}
				else
				{
					// probably too expensive to draw precisely over the cube
					// ForeColor(bgColor);
					// cubes[i]->solidCube(false);
					// draw a close square over the area instead after this
				}
			}
		}
		if (!wireFrame)
		{
			// use the cube update region to for blanking
			EraseRgn(cubeRgn);
		}
#endif

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
				cubes[i]->calculateBounds();
				if (doMove)
				{
					// reflection off the edges of the screen
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

					// reflection off front and back of the screen
					if (cubes[i]->getZ() > 100 && cubes[i]->velocity.z > 0)
					{
						cubes[i]->velocity.z *= -1;
					}
					else if (cubes[i]->getZ() < -400 && cubes[i]->velocity.z < 0)
					{
						cubes[i]->velocity.z *= -1;
					}
				}

				// render
				if (wireFrame)
					cubes[i]->draw(true);
				else
				{
					// prepare for depth sorting
					cubeDists[cubeAt] = cubes[i]->getZ();
					cubeIdx[cubeAt] = i;
					cubeAt++;
				}
			}
		}

		// render the cubes in depth order
		// depth sort the cubes here so that newer cubes are rendered on top
		bubbleSort(cubeDists, cubeIdx, cubeAt);
		for (i = 0; i < cubeAt; i++)
		{
			cubes[cubeIdx[i]]->solidCube(true);
		}
		cubeAt = 0; // reset for next loop

		// write TPF/FPU
		ForeColor(fgColor);
		writeTPF(buffer, TPF);
		MoveTo(7, yRes - 7);
		DrawString((unsigned char *)FPUbuffer);

		// copy back buffer to front, which will also trigger a display refresh
#ifdef USEOFFSCREEN
		// only update where the cube is
		SetEmptyRgn(updateRgn);
		for (i = 0; i < NUM_CUBES; i++)
		{
			if (activeCubes[i])
			{
				cubes[i]->roughBounds(cubeRgn, xRes, yRes);
				UnionRgn(updateRgn, cubeRgn, updateRgn);
			}
		}
		CopyRgn(updateRgn, cubeRgn);

		UnionRgn(updateRgn, tpfRgn, updateRgn);

		SetGWorld(onScreen, onscreenDevice);
		CopyBits(&(((GrafPtr)offScreen)->portBits),
				 &(((GrafPtr)onScreen)->portBits),
				 &(offScreen->portRect),
				 &(onScreen->portRect),
				 //  srcCopy, updateRgn); // copy just updated region
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
	DisposeRgn(cubeRgn);
	UnlockPixels(pixels);
	DisposeGWorld(offScreen);
#endif
	DisposeWindow(appWindow);
}

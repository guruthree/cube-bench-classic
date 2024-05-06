// a lot of safety checks are _not_ being done

#include <QDOffscreen.h>

#include "functions.h"
#include "cube.h"
#include "vectormath2.h"
#include "bubblesort.h"
#include "screenshot.h"

// how much smaller than the resolution to make the window
#define SHRINK 40

// use back buffer to render to, at the cost of some performance
#define USEOFFSCREEN

// clear the entire/canvase back buffer
// (instead of just the cube by writing over it)
// #define ERASECANVAS

// how many cubes we could be rendering
#define NUM_CUBES 10

void main()
{
	// find out about our hardware some
	SysEnvRec sys_info;
	SysEnvirons(curSysEnvVers, &sys_info);

#if mc68881
	// if we are compiled for fpu but don't have an FPU, quit
	// although we might still crash anyway, BasiliskII does at least :(
	if (!sys_info.hasFPU)
	{
		SysBeep(1);
		exit(0);
	}
#endif

	// for looping
	short i, j;

	// window & input variables
	short xRes, yRes;
	WindowPtr appWindow;
	Rect windowRect;
	Boolean running;
	EventRecord myEvent;
	char keyChar;

	// graphics variables
	GDHandle onscreenDevice;
	GWorldPtr onScreen;
#ifdef USEOFFSCREEN
	GWorldPtr offScreen;
	PixMapHandle pixels;
#endif
	RgnHandle updateRgn, tpfRgn, cubeRgn;
	Boolean one_bit; // change behaivour with colour depth

	// cube variables
	Cube *cubes[NUM_CUBES] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	Boolean activeCubes[NUM_CUBES] = {true, false, false, false, false, false, false, false, false, false};
	cubes[0] = new Cube(100);
	Cube *myCube = cubes[0]; // default cube for control
	short cubeAt = 0;
	float cubeDists[10];
	short cubeIdx[10];
	randSeed = TickCount();

	// cube display/interaction settings
	Boolean doRotate = true;
	Boolean doMove = false;
	Boolean wireFrame = true;
	Boolean invertColor = false;
	Boolean doBounce = false;

	// benchmark variables
	unsigned char TPF = 0;	// ticks per frame
	char buffer[20];		// for displaying stats
	unsigned long int last; // time of last frame (in ticks)
	float timeElapsed = 0;	// in milliseconds
	// apparently microseconds might not be supported everywhere?
	// https://68kmla.org/bb/index.php?threads/better-than-tickcount-timing.41563/
	// the alternative would be to use a time manager task to count its elapsed time, listing 3-6 in
	// https://developer.apple.com/library/archive/documentation/mac/pdf/Processes/Time_Manager.pdf
	UnsignedWide currentTimeW;
	Microseconds(&currentTimeW);
	float lastTime = MicrosecondToFloatMillis(&currentTimeW);
	float currentTime = lastTime;

	// colouring stuff
	long fgColor = blackColor;
	long bgColor = whiteColor;

	// initialisation
	InitToolbox();

	// determine FPU info
	char FPUbuffer[30] = "";
	getCPUandFPU(FPUbuffer, &sys_info);

	// convert current monitor resolution to window size by shrinking
	// GetGDevice was crashing on Mac SE + System 7, so disabled
	//	windowRect = (*(GetGDevice()))->gdRect;
	//	windowRect.top += SHRINK + 32;
	//	windowRect.bottom -= SHRINK;
	//	windowRect.left += SHRINK;
	//	windowRect.right -= SHRINK;

	// convert current monitor resolution to window size by shrinking
	// screenBits is a magic quickdraw global variable
	// sadly not not multimonitor compatible
	SetRect(&windowRect,
			screenBits.bounds.left + SHRINK,
			screenBits.bounds.top + SHRINK + 32,
			screenBits.bounds.right - SHRINK,
			screenBits.bounds.bottom - SHRINK);

	// calculate the window resolution
	xRes = windowRect.right - windowRect.left;
	yRes = windowRect.bottom - windowRect.top;

	// create window, sized by windowRect in absolute screen coordinates
	appWindow = NewWindow(0L, &windowRect, "\pCube Bench Classic",
						  true, movableDBoxProc, (WindowPtr)-1L, 1, 0);
	// 0 - auto assign memory, window size, window title, visible
	// window type, put window in front, have close box, refCon
	ShowWindow(appWindow); // bring to front
	SetPort(appWindow);	   // make the window the quickdraw graphics target

	// front buffer
	GetGWorld(&onScreen, &onscreenDevice);

	// get colour depth
	// treat all macs without color quickdraw as black and white
	one_bit = (*(*onscreenDevice)->gdPMap)->pixelSize == 1 || !sys_info.hasColorQD;

#ifdef USEOFFSCREEN
	// create back buffer
	// the first 0 is pixel depth 0, which reuses screen depth, this
	// is desired as the NewWindow lives in this same colour depth and
	// we need them to match in order to quickly copy between the two
	NewGWorld(&offScreen, 0, &(appWindow->portRect), NULL, NULL, 0);
	// "ctSeed slamming" to avoid having to remap colours on blit
	// GetGDevice was crashing on Mac SE + System 7, so disabled
	//	(*((*(offScreen->portPixMap))->pmTable))->ctSeed =
	//		(*((*((*(GetGDevice()))->gdPMap))->pmTable))->ctSeed;
	// we don't need to slam anyway as by passing NULL to ctTable we're
	// using the default color table for the pixel depth, which should
	// match what NewWindow used when creating its on screen window
	// get memory of back buffer & lock to keep from being destroyed
	pixels = GetGWorldPixMap(offScreen);
	NoPurgePixels(pixels);
	LockPixels(pixels);
	// might have been able to just use a Picture instead?
	// (although images don't support all features of a world)

	// make sure the back buffer is clear
	SetGWorld(offScreen, NULL);
	EraseRect(&appWindow->portRect);
	SetGWorld(onScreen, onscreenDevice);
#endif

	// screen area where TPF is drawn
	tpfRgn = NewRgn();
	SetRectRgn(tpfRgn, 0, 0, 100, 45);
	// screen area where FPU is drawn
	updateRgn = NewRgn();
	SetRectRgn(updateRgn, 0, yRes - 40, 80, yRes);
	// combine in tpfRgn
	UnionRgn(tpfRgn, updateRgn, tpfRgn);
	cubeRgn = NewRgn(); // screen space with cubes in it

	// SysBeep(1); // init OK
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
				case '`':  // back tick
				case 0x1b: // escape
				case '\r': // carraige return (enter)
					running = false;
					break;

				// uiojkl to control rotation
				case 'j':
					myCube->rotate(Vector3(-0.1, 0.0, 0.0));
					break;
				case 'l':
					myCube->rotate(Vector3(+0.1, 0.0, 0.0));
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

				// wasdqe to move on screen
				case 'a':
					myCube->translate(Vector3(-10, 0.0, 0.0));
					break;
				case 'd':
					myCube->translate(Vector3(+10, 0.0, 0.0));
					break;
				case 'w':
					myCube->translate(Vector3(0.0, -10, 0.0));
					break;
				case 's':
					myCube->translate(Vector3(0.0, +10, 0.0));
					break;
				case 'q':
					myCube->translate(Vector3(0.0, 0.0, -10));
					break;
				case 'e':
					myCube->translate(Vector3(0.0, 0.0, +10));
					break;

				// WASDQE to control movement speed
				case 'A':
					myCube->velocity.x -= 0.1f;
					break;
				case 'D':
					myCube->velocity.x += 0.1f;
					break;
				case 'W':
					myCube->velocity.y -= 0.1f;
					break;
				case 'S':
					myCube->velocity.y += 0.1f;
					break;
				case 'Q':
					myCube->velocity.z -= 0.1f;
					break;
				case 'E':
					myCube->velocity.z += 0.1f;
					break;

				// stop/start auto rotation
				case 'n':
				case 'N':
					doRotate = !doRotate;
					break;

				// stop/start translation movement
				case 'm':
				case 'M':
					doMove = !doMove;
					break;

				// stop/start all movement
				case ' ': // space
					if ((doRotate || doMove) && doRotate != doMove)
					{
						doRotate = false;
						doMove = false;
					}
					else if ((!doRotate || !doMove) && doRotate != doMove)
					{
						doRotate = true;
						doMove = true;
					}
					else
					{
						doRotate = !doRotate;
						doMove = !doMove;
					}
					break;

				// stop/start bounce collisions
				case 'b':
				case 'B':
					doBounce = !doBounce;
					break;

				// reset cubes to their initial positions
				case 'r':
					// TODO
					break;

				// randomise rotation & speed
				case 'R':
					randomiseCubes(cubes, activeCubes, xRes, yRes, NUM_CUBES);
					break;

				// black background, white text (invert)
				case 'v':
				case 'V':
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
					//	break;

				// erase (clear) buffer
				case 'c':
				case 'C':
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
					keyChar -= 48;	  // go from char to short
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
						cubes[keyChar] = new Cube(50);
						activeCubes[keyChar] = true;
					}
					else
						activeCubes[keyChar] = !activeCubes[keyChar];
					if (activeCubes[keyChar])
						myCube = cubes[keyChar];
					break;

				// screen shot - only caps so it's hard to do on accident
				case 'P':
					// TODO
					takeScreenshot(offScreen);
					// reset timers after coming back from screen shot to not mess up stats
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
					else if (cubes[i]->getZ() < -1000 && cubes[i]->velocity.z < 0)
					{
						cubes[i]->velocity.z *= -1;
					}
				}

				// render
				if (wireFrame)
				{
					if (!one_bit)
					{
						// draw will set colours
						cubes[i]->draw(true);
					}
					else
					{
						// when in one_bit mode, we need to paint in white
						ForeColor(fgColor);
						cubes[i]->draw(false);
					}
				}
				else
				{
					// prepare for depth sorting
					cubeDists[cubeAt] = 1000 - cubes[i]->getZ();
					cubeIdx[cubeAt] = i;
					cubeAt++;
				}
			}
		}

		if (doBounce)
		{
			for (i = 0; i < NUM_CUBES; i++)
			{
				for (j = i + 1; j < NUM_CUBES; j++)
				{
					if (activeCubes[i] && activeCubes[j])
					{
						// bounce if:
						// cubes are close to each other
						// cubes are headed towards each other
						// based on screen coordinates
						// inverting velocities

						// TODO bounce code
						// sum(centre.^2 - centre.^2) < 100?
					}
				}
			}
		}

		// render the cubes in depth order
		// depth sort the cubes here so that newer cubes are rendered on top
		bubbleSort(cubeDists, cubeIdx, cubeAt);
		for (i = 0; i < cubeAt; i++)
		{
			if (!one_bit)
			{
				// solidCube() will set colours before calling solidFace()
				cubes[cubeIdx[i]]->solidCube(true, one_bit);
			}
			else
			{
				// when in one_bit mode, we need to paint in white
				ForeColor(fgColor);
				// cubes[i]->draw(false); // makes it much easier to see, but costs more
				cubes[cubeIdx[i]]->solidCube(false, one_bit);
			}
		}
		cubeAt = 0; // reset for next loop

		// write TPF, stats, & CPU/FPU situation
		ForeColor(fgColor);
		writeStats(buffer, TPF, timeElapsed);
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

		// benchmark stuff
		TPF = TickCount() - last;
		Microseconds(&currentTimeW);
		currentTime = MicrosecondToFloatMillis(&currentTimeW);
		timeElapsed = currentTime - lastTime;
		lastTime = currentTime;

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

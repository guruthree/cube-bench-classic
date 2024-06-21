// a lot of safety checks are _not_ being done

#include <QDOffscreen.h>
#include <time.h>

#include "config.h"
#include "functions.h"
#include "cube.h"
#include "vectormath2.h"
#include "bubblesort.h"
#include "screenshot.h"
#include "statistics.h"

void main()
{
	// find out about our hardware some
	SysEnvRec sys_info;
	SysEnvirons(curSysEnvVers, &sys_info);

	// check for at least System 7
	if ((sys_info.systemVersion & 0xFF00) >> 8 < 7)
	{
		SysBeep(1);
		SysBeep(1);
		Delay(60, NULL);
		exit(0);
	}

#if mc68881
	// if we are compiled for fpu but don't have an FPU, quit
	// although we might still crash anyway, BasiliskII does at least :(
	if (!sys_info.hasFPU)
	{
		SysBeep(1);
		Delay(30, NULL);
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
	RgnHandle updateRgn, tpfRgn, cubeRgn, helpRgn;
	Boolean one_bit; // change behaviour with colour depth
	Boolean showHelp = false;

	// cube variables
	Cube *cubes[NUM_CUBES] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	for (i = 0; i < NUM_CUBES; i++)
	{
		cubes[i] = new Cube(100);
	}
	Boolean activeCubes[NUM_CUBES] = {true, false, false, false, false, false, false, false, false, false};
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
	Statistics fps = Statistics(SHORT_STATS, false);
	Statistics frametimes = Statistics(SHORT_STATS, false);
#ifdef LONG_STATS
	Boolean benchmarking = false;
	Statistics frametimes_long = Statistics(LONG_STATS, true);
#endif

	// colouring stuff
	long fgColor = blackColor;
	long bgColor = whiteColor;

	// initialisation
	InitToolbox();

	// determine FPU info
	char FPUbuffer[64] = "";
	getCPUandFPU(FPUbuffer, &sys_info);

	// buffer for date and time
	char TIMEbuffer[50] = "";
	time_t nt;

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

	// ensure the vertical resolution is a multiple of 2
	if (yRes % 2 != 0)
	{
		windowRect.bottom--;
		yRes--;
	}

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
	SetRectRgn(tpfRgn, 0, 0, 160, 15 + 13 * 4 + 1);
	// screen area where FPU is drawn
	updateRgn = NewRgn();
	SetRectRgn(updateRgn, 0, yRes - 7 - 13 - 5, 160, yRes);
	// combine in tpfRgn
	UnionRgn(tpfRgn, updateRgn, tpfRgn);
	cubeRgn = NewRgn(); // screen space with cubes in it
	// screen space where help is drawn
	helpRgn = NewRgn();
	SetRectRgn(helpRgn, xRes - HELP_OFFSET, 0, xRes, 15 + 13 * 14 + 1);

	// put all the cubes in their programmed positions
	resetCubes(cubes, activeCubes, xRes, yRes);

	// SysBeep(1); // init OK
	running = true;
	while (running == true)
	{
		last = TickCount();

		// sleep is 0 to be greedy and try and not let things happen in the background
		if (WaitNextEvent(everyEvent, &myEvent, 0, NULL))
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
#ifdef LONG_STATS
				// if benchmarking pressing any key cancels
				// once complete pressing any key other than a save key resets
				if (benchmarking || (frametimes_long.completed && keyChar != 'P' && keyChar != 'T'))
				{
					benchmarking = false;
					frametimes_long.reset();
				}
#endif
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

				// enable/disable bounce collisions
				case 'x':
				case 'X':
					doBounce = !doBounce;
					break;

				// reset cubes to their initial positions
				case 'r':
					doRotate = true;
					doMove = false;
					doBounce = false;
					resetCubes(cubes, activeCubes, xRes, yRes);
					break;

				// randomise rotation & speed
				case 'R':
					randomiseCubes(cubes, activeCubes, xRes, yRes);
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
					takeScreenshot(offScreen);
					// reset timers after coming back from screen shot to not mess up stats
					break;

				// show/hide help
				case 'h':
				case 'H':
					showHelp = !showHelp;
					if (!showHelp)
					{
#ifdef USEOFFSCREEN
						SetGWorld(offScreen, NULL);
#endif
						EraseRgn(helpRgn);
#ifdef USEOFFSCREEN
						SetGWorld(onScreen, onscreenDevice);
						CopyBits(&(((GrafPtr)offScreen)->portBits),
								 &(((GrafPtr)onScreen)->portBits),
								 &(offScreen->portRect),
								 &(onScreen->portRect),
								 srcCopy, NULL);
#endif
					}
					break;

#ifdef LONG_STATS
				// run a benchmark - bits of code here copy-pasted from above
				case 'b':
				case 'B':
					benchmarking = true;
					frametimes_long.reset();
					// reset cubes
					doRotate = true;
					doMove = false;
					doBounce = false;
					resetCubes(cubes, activeCubes, xRes, yRes);
					// clear buffer
#ifdef USEOFFSCREEN
					SetGWorld(offScreen, NULL);
#endif // USEOFFSCREEN
					EraseRect(&appWindow->portRect);
#ifdef USEOFFSCREEN
					SetGWorld(onScreen, onscreenDevice);
					CopyBits(&(((GrafPtr)offScreen)->portBits),
							 &(((GrafPtr)onScreen)->portBits),
							 &(offScreen->portRect),
							 &(onScreen->portRect),
							 srcCopy, NULL);
#endif // USEOFFSCREEN
					break;

				// save stats to a file
				case 'T':
					frametimes_long.writeToFile("\pStats.txt", offScreen, FPUbuffer, activeCubes, wireFrame, TIMEbuffer);
#endif // LONG_STATS

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
		// clear help text
		if (showHelp)
		{
			EraseRgn(helpRgn);
		}

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

		// record stats
		frametimes.addValue(timeElapsed);
		fps.addValue((1.0 / (timeElapsed / 1000.0)));
#ifdef LONG_STATS
		if (benchmarking)
		{
			frametimes_long.addValue(timeElapsed);
			if (frametimes_long.completed)
			{
				benchmarking = false;
				// don't move anything after the benchmark is finished so that what's on
				// screen reflects the benchmark settings - although technically this is
				// one frame after the last included in the benchmark...
				doRotate = false;
				doMove = false;
				doBounce = false;
			}
		}
#endif

		// write TPF, stats, & CPU/FPU situation
		ForeColor(fgColor);
		writeTPF(TPF); // MoveTo(7, 15); before writing
		fps.write(15 + 13, "FPS");
		frametimes.write(15 + 13 * 2, "FT");
#ifdef LONG_STATS
		if (benchmarking || frametimes_long.completed)
		{
			frametimes_long.write(15 + 13 * 3, "Benchmark");
		}
#endif
		MoveTo(7, yRes - 7 - 13);
		DrawString((unsigned char *)FPUbuffer);
		nt = time(NULL);
		strftime(TIMEbuffer, 30, "%Y/%m/%d %H:%M:%S", localtime(&nt));
		CtoPstr(TIMEbuffer);
		MoveTo(7, yRes - 7);
		DrawString((unsigned char *)TIMEbuffer);

		if (showHelp)
		{
			// display the help message
			writeHelp(xRes - HELP_OFFSET + 1);
		}

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
		if (showHelp)
		{
			UnionRgn(updateRgn, helpRgn, updateRgn);
		}

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

// a lot of safety checks are _not_ being done

#include <QDOffscreen.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "vectormath2.h"
#include "bubblesort.h"

// how much smaller than the resolution to make the window
//#define SHRINK 40
// a smaller resolution is needed for the emulator
#define SHRINK 160

// use back buffer to render to, at the cost of some performance
#define USEOFFSCREEN

// clear just the cube by writing over it
// when unset clear the entire back buffer
#define ERASECUBE

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

// rotate the original cube to a new position
void rotateCube(Vector3 *cube, Vector3 *rotatedCube, Vector3 *centre, float xangle, float yangle, float zangle, int xRes, int yRes)
{
	int i;
	Matrix3 rot = Matrix3::getRotationMatrix(xangle, yangle, zangle);
	for (i = 0; i < 8; i++)
	{
		rotatedCube[i] = rot.preMultiply(cube[i]);
		rotatedCube[i] = rotatedCube[i].add(*centre);
		rotatedCube[i] = rotatedCube[i].scale(40.0f / (-rotatedCube[i].z / 8.0f + 40.0f));
		rotatedCube[i].x = rotatedCube[i].x + xRes / 2;
		rotatedCube[i].y = rotatedCube[i].y + yRes / 2;
	}
}

// render the cube
void drawCube(Vector3 *rotatedCube, Boolean color)
{
	// front face
	if (color)
	{
		ForeColor(redColor);
	}
	MoveTo(rotatedCube[0].x, rotatedCube[0].y);
	LineTo(rotatedCube[1].x, rotatedCube[1].y);
	LineTo(rotatedCube[2].x, rotatedCube[2].y);
	LineTo(rotatedCube[3].x, rotatedCube[3].y);
	LineTo(rotatedCube[0].x, rotatedCube[0].y);

	// back face
	if (color)
	{
		ForeColor(blueColor);
	}
	MoveTo(rotatedCube[4].x, rotatedCube[4].y);
	LineTo(rotatedCube[5].x, rotatedCube[5].y);
	LineTo(rotatedCube[6].x, rotatedCube[6].y);
	LineTo(rotatedCube[7].x, rotatedCube[7].y);
	LineTo(rotatedCube[4].x, rotatedCube[4].y);
	
	// connecting bits
	if (color)
	{
		ForeColor(greenColor);
	}
	MoveTo(rotatedCube[0].x, rotatedCube[0].y);
	LineTo(rotatedCube[4].x, rotatedCube[4].y);
	MoveTo(rotatedCube[1].x, rotatedCube[1].y);
	LineTo(rotatedCube[5].x, rotatedCube[5].y);
	MoveTo(rotatedCube[2].x, rotatedCube[2].y);
	LineTo(rotatedCube[6].x, rotatedCube[6].y);
	MoveTo(rotatedCube[3].x, rotatedCube[3].y);
	LineTo(rotatedCube[7].x, rotatedCube[7].y);
}

// filled cube
void solidCube(Vector3 *rotatedCube, Boolean color)
{
	PolyHandle poly;

	// front face
	if (color)
	{
		ForeColor(redColor);
	}
	poly = OpenPoly();
	MoveTo(rotatedCube[0].x, rotatedCube[0].y);
	LineTo(rotatedCube[1].x, rotatedCube[1].y);
	LineTo(rotatedCube[2].x, rotatedCube[2].y);
	LineTo(rotatedCube[3].x, rotatedCube[3].y);
	LineTo(rotatedCube[0].x, rotatedCube[0].y);
	ClosePoly();
	PaintPoly(poly);
	KillPoly(poly);

	// back face
	if (color)
	{
		ForeColor(blueColor);
	}
	poly = OpenPoly();
	MoveTo(rotatedCube[4].x, rotatedCube[4].y);
	LineTo(rotatedCube[5].x, rotatedCube[5].y);
	LineTo(rotatedCube[6].x, rotatedCube[6].y);
	LineTo(rotatedCube[7].x, rotatedCube[7].y);
	LineTo(rotatedCube[4].x, rotatedCube[4].y);
	ClosePoly();
	PaintPoly(poly);
	KillPoly(poly);
	

	// side face one
	if (color)
	{
		ForeColor(greenColor);
	}
	poly = OpenPoly();
	MoveTo(rotatedCube[0].x, rotatedCube[0].y);
	LineTo(rotatedCube[4].x, rotatedCube[4].y);
	LineTo(rotatedCube[5].x, rotatedCube[5].y);
	LineTo(rotatedCube[1].x, rotatedCube[1].y);
	LineTo(rotatedCube[0].x, rotatedCube[0].y);
	ClosePoly();
	PaintPoly(poly);
	KillPoly(poly);
	
	// side face two
	if (color)
	{
		ForeColor(magentaColor);
	}
	poly = OpenPoly();
	MoveTo(rotatedCube[0].x, rotatedCube[0].y);
	LineTo(rotatedCube[4].x, rotatedCube[4].y);
	LineTo(rotatedCube[7].x, rotatedCube[7].y);
	LineTo(rotatedCube[3].x, rotatedCube[3].y);
	LineTo(rotatedCube[0].x, rotatedCube[0].y);	
	ClosePoly();
	PaintPoly(poly);
	KillPoly(poly);
	
	// side face three
	if (color)
	{
		ForeColor(cyanColor);
	}
	poly = OpenPoly();
	MoveTo(rotatedCube[2].x, rotatedCube[2].y);
	LineTo(rotatedCube[6].x, rotatedCube[6].y);
	LineTo(rotatedCube[7].x, rotatedCube[7].y);
	LineTo(rotatedCube[3].x, rotatedCube[3].y);
	LineTo(rotatedCube[2].x, rotatedCube[2].y);
	ClosePoly();
	PaintPoly(poly);
	KillPoly(poly);	
	
	// side face four
	if (color)
	{
		ForeColor(yellowColor);
	}
	poly = OpenPoly();	
	MoveTo(rotatedCube[2].x, rotatedCube[2].y);
	LineTo(rotatedCube[6].x, rotatedCube[6].y);
	LineTo(rotatedCube[5].x, rotatedCube[5].y);
	LineTo(rotatedCube[1].x, rotatedCube[1].y);
	LineTo(rotatedCube[2].x, rotatedCube[2].y);	
	ClosePoly();
	PaintPoly(poly);
	KillPoly(poly);	
}

//	int frontFace[5] = {0, 1, 2, 3, 0};
//	int backFace[5] = {4, 5, 6, 7, 4};
//	int sideFaceOne[5] = {0, 4, 5, 1, 0};
//	int sideFaceTwo[5] = {0, 4, 7, 3, 0};
//	int sideFaceThree[5] = {2, 6, 7, 3, 2};
//	int sideFaceFour[5] = {2, 6, 5, 1, 2};
//	long colors[6] = {redColor, blueColor, greenColor, magentaColor, cyanColor, yellowColor};

void solidFace(Vector3 *rotatedCube, int *verticies)
{
	int j;
	PolyHandle poly = OpenPoly();
	MoveTo(rotatedCube[verticies[0]].x, rotatedCube[verticies[0]].y);
	for (j = 1; j < 5; j++)
	{
		LineTo(rotatedCube[verticies[j]].x, rotatedCube[verticies[j]].y);
	}
	ClosePoly();
	PaintPoly(poly);
	KillPoly(poly);
}

void solidCube2(Vector3 *rotatedCube, int faces[][5], Boolean color, long *colors)
{
	// we need to depth sort to try and get rendering right
	// only need to draw the closest 3 sides?
	int i, j;
	// just need one vector3 to use for temp then reset?
	Vector3 faceCenters[6] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
	float averageDepths[6] = {0, 0, 0, 0, 0, 0};
	float indexes[6] = {0, 1, 2, 3, 4, 5};
	
	for (i = 0; i < 6; i++)
	{
		for (j = 1; j < 5; j++)
		{
			faceCenters[i] = faceCenters[i].add(rotatedCube[faces[i][j]]);
		}
		averageDepths[i] = faceCenters[i].scale(0.25).z;
	}
	
	for (i = 0; i < 6; i++)
	{
		if (color)
		{
			ForeColor(colors[i]);
		}
		solidFace(rotatedCube, faces[i]);
//		return;
	}
}

// identify the region on screen that the cube is using
void cubeBounds(Vector3 *cube, RgnHandle rgn)
{
	int i;
	int left = 1000, right = 0, top = 1000, bottom = 0;

	for (i = 0; i < 8; i++)
	{
		if (cube[i].x < left) left = cube[i].x;
		if (cube[i].x > right) right = cube[i].x;
			
		if (cube[i].y < top) top = cube[i].y;
		if (cube[i].y > bottom) bottom = cube[i].y;
	}

	// this isn't enough when the cube is spinning fast
	SetRectRgn(rgn, left-3, top-3, right+3, bottom+3);
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
	float size = 100;
	float xangle = 30, yangle = 0, zangle = 45;
	float dxangle = 0.01, dyangle = 0.005, dzangle = 0.01;
	float olddxangle = dxangle, olddyangle = dyangle, olddzangle = dzangle;
	Vector3 cube[8] = {
		{-size / 2, -size / 2,  size / 2},
		{ size / 2, -size / 2,  size / 2},
		{ size / 2,  size / 2,  size / 2},
		{-size / 2,  size / 2,  size / 2},
		{-size / 2, -size / 2, -size / 2},
		{ size / 2, -size / 2, -size / 2},
		{ size / 2,  size / 2, -size / 2},
		{-size / 2,  size / 2, -size / 2}};
	int faces[6][5] = {
		{0, 1, 2, 3, 0},
		{4, 5, 6, 7, 4},
		{0, 4, 5, 1, 0},
		{0, 4, 7, 3, 0},
		{2, 6, 7, 3, 2},
		{2, 6, 5, 1, 2}};
	long colors[6] = {redColor, blueColor, greenColor, magentaColor, cyanColor, yellowColor};
	int i;
	Vector3 centre = {0, 0, 0};
	Vector3 rotatedCube[8];
	Boolean doSolid = false;

	// benchmark variables
	unsigned char TPF = 0; // ticks per frame
	char buffer[100]; // for displaying TPF
	unsigned long int last; // time of last frame

	// initialisation
	InitToolbox();

	// get current monitor resolution
	windowRect = (*(GetGDevice()))->gdRect;
	// convert to window size by shrinking
	windowRect.top += SHRINK;
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
	DisposeRgn(updateRgn); // dispose so we can reuse later

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

						// r randomize rotation & speed
						
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
							doSolid = !doSolid;
						 	break;
						
						// b black background, white text

						default:
						 	break;
					}
					break;

				default:
					break;
			}
		}

		// process cube
		xangle += dxangle;
		yangle += dyangle;
		zangle += dzangle;
		if (xangle >= 360) xangle -= 360;
		if (yangle >= 360) yangle -= 360;
		if (zangle >= 360) zangle -= 360;
#ifndef ERASECUBE
		rotateCube(cube, rotatedCube, &centre, xangle, yangle, zangle, xRes, yRes);
#endif

		// render
#ifdef USEOFFSCREEN
		SetGWorld(offScreen, NULL);
#endif
		// erase the bound of port rect on the current port
#ifndef ERASECUBE
		EraseRect(&appWindow->portRect);
#else // erase just the pixels of the cube
		// clear TPF/FPU
#ifdef USEOFFSCREEN
		EraseRgn(tpfRgn);
#endif

		// draw over old cube
		ForeColor(whiteColor);
		drawCube(rotatedCube, false);

		// rotate & draw new cube
		rotateCube(cube, rotatedCube, &centre, xangle, yangle, zangle, xRes, yRes);
#endif
		if (!doSolid)
			drawCube(rotatedCube, true);
		else
//			solidCube(rotatedCube, true);
			solidCube2(rotatedCube, faces, true, colors);

		// write TPF/FPU
		ForeColor(blackColor);
		writeTPF(buffer, TPF);
		MoveTo(7, yRes - 7);
		#if mc68881
			DrawString("\pFPU");
		#else
			DrawString("\pNO FPU");
		#endif

		// copy back buffer to front, which will also trigger a display refresh
#ifdef USEOFFSCREEN
		// only update where the cube is
		updateRgn = NewRgn();
		cubeBounds(rotatedCube, updateRgn);
		UnionRgn(updateRgn, tpfRgn, updateRgn);

		SetGWorld(onScreen, onscreenDevice);
		CopyBits(&(((GrafPtr)offScreen)->portBits),
				 &(((GrafPtr)onScreen)->portBits),
				 &(offScreen->portRect),
				 &(onScreen->portRect),
				 srcCopy, updateRgn);
				 
		DisposeRgn(updateRgn);
#endif
		TPF = TickCount() - last;

	} // while running

#ifdef USEOFFSCREEN
	DisposeRgn(tpfRgn);
	UnlockPixels(pixels);
	DisposeGWorld(offScreen);
#endif
	DisposeWindow(appWindow);
}

// a lot of safety checks are _not_ being done

#include <QDOffscreen.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "vectormath2.h"

#define WINDOWX 40
#define WINDOWY 40+32
//#define XRESOLUTION 340
//#define YRESOLUTION 240
#define XRESOLUTION 560
#define YRESOLUTION 368

#define USEOFFSCREEN

void InitToolbox(void);

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

void rotateCube(Vector3 *cube, Vector3 *rotatedCube, Vector3 *centre, float xangle, float yangle, float zangle)
{
	int i;
	Matrix3 rot = Matrix3::getRotationMatrix(xangle, yangle, zangle);
	for (i = 0; i < 8; i++)
	{
		rotatedCube[i] = rot.preMultiply(cube[i]);
		rotatedCube[i] = rotatedCube[i].add(*centre);
		rotatedCube[i] = rotatedCube[i].scale(40.0f / (-rotatedCube[i].z / 8.0f + 40.0f));
		rotatedCube[i].x = rotatedCube[i].x + XRESOLUTION / 2;
		rotatedCube[i].y = rotatedCube[i].y + YRESOLUTION / 2;
	}
}

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

void cubeBounds(Vector3 *rotatedCube, RgnHandle rgn)
{
	int i;
	int left = 1000, right = 0, top = 1000, bottom = 0;

	for (i = 0; i < 8; i++)
	{
		if (rotatedCube[i].x < left)
			left = rotatedCube[i].x;
		if (rotatedCube[i].x > right)
			right = rotatedCube[i].x;
			
		if (rotatedCube[i].y < top)
			top = rotatedCube[i].y;
		if (rotatedCube[i].y > bottom)
			bottom = rotatedCube[i].y;
	}

	SetRectRgn(rgn, left-10, top-10, right+10, bottom+10);
}

void writeTPF(char buffer[], unsigned char TPF)
{
	sprintf(buffer, "TPF: %hu", TPF);
	CtoPstr(buffer);
	ForeColor(blackColor);
	MoveTo(7, 15);
	DrawString((unsigned char *)buffer);
}


void main()
{
	// window variables
	WindowPtr appWindow;
	Rect windowRect;
	Boolean running;
	EventRecord myEvent;

	// graphics variables
#ifdef USEOFFSCREEN
	GDHandle onscreenDevice;
	GWorldPtr onScreen, offScreen; //, mask;
	PixMapHandle pixels; //, maskpixels;
//	Rect tpfRect;
	RgnHandle updateRgn, tpfRgn;
#endif

	// cube variables
	float size = 100;
	float xangle = 30, yangle = 0, zangle = 45;
	float dxangle = 0.01, dyangle = 0.0, dzangle = 0.01;
	Vector3 cube[8] = {
		{-size / 2, -size / 2,  size / 2},
		{ size / 2, -size / 2,  size / 2},
		{ size / 2,  size / 2,  size / 2},
		{-size / 2,  size / 2,  size / 2},
		{-size / 2, -size / 2, -size / 2},
		{ size / 2, -size / 2, -size / 2},
		{ size / 2,  size / 2, -size / 2},
		{-size / 2,  size / 2, -size / 2}};
	int i;
	Vector3 centre = {0, 0, 0};
	Vector3 rotatedCube[8];

	// benchmark variables
	unsigned char TPF = 0; // ticks per frame
	char buffer[100];
	InitToolbox();
	unsigned long int last;

	SetRect(&windowRect, WINDOWX, WINDOWY, WINDOWX + XRESOLUTION, WINDOWY + YRESOLUTION);
	appWindow = NewWindow(0L, &windowRect, "\pSpinning Cube Benchmark", true, movableDBoxProc, (WindowPtr)-1L, 1, 0);
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
	
//	NewGWorld(&mask, 1, &(appWindow->portRect), NULL, NULL, 0);
//	maskpixels = GetGWorldPixMap(offScreen);
//	LockPixels(maskpixels);
	
//	SetRect(&tpfRect, 0, 0, 80, 40); // for covering up old TPF

tpfRgn = NewRgn();
	SetRectRgn(tpfRgn, 0, 0, 80, 40);
	
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
		rotateCube(cube, rotatedCube, &centre, xangle, yangle, zangle);

		// render
#ifdef USEOFFSCREEN
		SetGWorld(offScreen, NULL);
#endif
		// erase the bound of port rect on the current port
		EraseRect(&appWindow->portRect);
//		EraseRect(&tpfRect);
		drawCube(rotatedCube, true);
		writeTPF(buffer, TPF);
		
		ForeColor(blackColor);
		MoveTo(7, YRESOLUTION - 7);
		#if mc68881
			DrawString("\pFPU");
		#else
			DrawString("\pNO FPU");
		#endif
		
//		SetGWorld(mask, NULL);
//		EraseRect(&appWindow->portRect);
//		InvertRect(&tpfRect);
//		drawCube(rotatedCube, false);

updateRgn = NewRgn();
//RectRgn(updateRgn, &tpfRect);
cubeBounds(rotatedCube, updateRgn);
UnionRgn(updateRgn, tpfRgn, updateRgn);

		// copy back buffer to front, which will also trigger a display refresh
#ifdef USEOFFSCREEN
		SetGWorld(onScreen, onscreenDevice);
//		EraseRect(&appWindow->portRect);
		CopyBits(&(((GrafPtr)offScreen)->portBits),
				 &(((GrafPtr)onScreen)->portBits),
				 &(offScreen->portRect),
				 &(onScreen->portRect),
				 srcCopy, updateRgn);
//				 srcCopy, NULL);
//		CopyMask(&(((GrafPtr)offScreen)->portBits),
//				 &(((GrafPtr)mask)->portBits),
//				 &(((GrafPtr)onScreen)->portBits),
//				 &(offScreen->portRect),
//				 &(mask->portRect),
//				 &(onScreen->portRect));
#endif


DisposeRgn(updateRgn);

		TPF = TickCount() - last;

	} // while running

#ifdef USEOFFSCREEN
	DisposeRgn(tpfRgn);
//	UnlockPixels(maskpixels);
//	DisposeGWorld(mask);
	UnlockPixels(pixels);
	DisposeGWorld(offScreen);
#endif
	DisposeWindow(appWindow);
}

// a lot of safety checks are _not_ being done

#include <QDOffscreen.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "vectormath2.h"

#define WINDOWX 40
#define WINDOWY 40+32
#define XRESOLUTION 340
#define YRESOLUTION 240

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

void drawCube(Vector3 *rotatedCube)
{
	// front face
	MoveTo(rotatedCube[0].x, rotatedCube[0].y);
	LineTo(rotatedCube[1].x, rotatedCube[1].y);
	LineTo(rotatedCube[2].x, rotatedCube[2].y);
	LineTo(rotatedCube[3].x, rotatedCube[3].y);
	LineTo(rotatedCube[0].x, rotatedCube[0].y);

	// back face
	MoveTo(rotatedCube[4].x, rotatedCube[4].y);
	LineTo(rotatedCube[5].x, rotatedCube[5].y);
	LineTo(rotatedCube[6].x, rotatedCube[6].y);
	LineTo(rotatedCube[7].x, rotatedCube[7].y);
	LineTo(rotatedCube[4].x, rotatedCube[4].y);
	
	// connecting bits
	MoveTo(rotatedCube[0].x, rotatedCube[0].y);
	LineTo(rotatedCube[4].x, rotatedCube[4].y);
	MoveTo(rotatedCube[1].x, rotatedCube[1].y);
	LineTo(rotatedCube[5].x, rotatedCube[5].y);
	MoveTo(rotatedCube[2].x, rotatedCube[2].y);
	LineTo(rotatedCube[6].x, rotatedCube[6].y);
	MoveTo(rotatedCube[3].x, rotatedCube[3].y);
	LineTo(rotatedCube[7].x, rotatedCube[7].y);
}

void writeTPF(char buffer[], unsigned char TPF)
{
	sprintf(buffer, "TPF: %hu", TPF);
	CtoPstr(buffer);
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
	GDHandle onscreenDevice;
	GWorldPtr onScreen, offScreen;
	PixMapHandle pixels;

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

	// front buffer
	GetGWorld(&onScreen, &onscreenDevice);
	// create back buffer
	NewGWorld(&offScreen, 0, &(appWindow->portRect), NULL, NULL, 0);
	// get memory of back buffer & lock to keep from being destroyed
	pixels = GetGWorldPixMap(offScreen);
	LockPixels(pixels);

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
		SetGWorld(offScreen, NULL);
		EraseRect(&appWindow->portRect);
		drawCube(rotatedCube);
		writeTPF(buffer, TPF);
		
		MoveTo(7, YRESOLUTION - 7);
		#if mc68881
			DrawString("\pFPU");
		#else
			DrawString("\pNO FPU");
		#endif

		// copy back buffer to front, which will also trigger a display refresh
		SetGWorld(onScreen, onscreenDevice);
		CopyBits(&(((GrafPtr)offScreen)->portBits),
				 &(((GrafPtr)onScreen)->portBits),
				 &(offScreen->portRect),
				 &(onScreen->portRect),
				 srcCopy, NULL);

		TPF = TickCount() - last;

	} // while running

	UnlockPixels(pixels);
	DisposeGWorld(offScreen);
	DisposeWindow(appWindow);
}

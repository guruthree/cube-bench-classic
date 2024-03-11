// a lot of safety checks are _not_ being done

#include <QDOffscreen.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "vectormath2.h"

#define XRESOLUTION 340
#define YRESOLUTION 240

void InitToolbox(void);

void InitToolbox()
{
	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	FlushEvents(everyEvent,0);
	TEInit();
	InitDialogs(0L);
	InitCursor();
}

void main()
{
	WindowPtr newWindow;
	Rect windowRect;
	Boolean running, result;
	EventRecord myEvent;
	
	unsigned long int count;
	unsigned char smallint;
	char text[100];
	
	GDHandle onscreenDevice;
	GWorldPtr onscreen, offscreen;
	PixMapHandle pixels;
	
	InitToolbox();
	
	
	SetRect(&windowRect, 40, 40+32, XRESOLUTION, YRESOLUTION);	
	newWindow = NewWindow(0L, &windowRect, "\plines", true, movableDBoxProc, (WindowPtr) -1L, 1, 0);
	ShowWindow(newWindow); // bring to front
	SetPort(newWindow); // make the window the quickdraw graphics target

	// front buffer
	GetGWorld(&onscreen, &onscreenDevice);
	// create back buffer
	NewGWorld(&offscreen, 0, &(newWindow->portRect), NULL, NULL, 0);
	// get memory of back buffer & lock to keep from being destroyed
	pixels = GetGWorldPixMap(offscreen);
	LockPixels(pixels);
	
	running = true;
	while ( running == true ) {

		result = WaitNextEvent(everyEvent, &myEvent, 1, NULL);
		if (result) {
			switch (myEvent.what) {
			
				case activateEvt:
					if ((myEvent.modifiers & activeFlag) == 0)  {
						running = false;
					}
					break;

				default:
					break;
			}					
		}
		
		count++;
		smallint = count; // cut things off and get only the <255 bit

		SetGWorld(offscreen, NULL);
		EraseRect(&newWindow->portRect);
		MoveTo(20,20);
		sprintf(text, "COUNT: %hu", smallint);
		CtoPstr(text);
		DrawString((unsigned char*)text);
		
		SetGWorld(onscreen, onscreenDevice);
		CopyBits(&(((GrafPtr)offscreen)->portBits), 
				 &(((GrafPtr)onscreen)->portBits), 
				 &(offscreen->portRect),
				 &(onscreen->portRect),
				 srcCopy, NULL);
		
	} // while running

	UnlockPixels(pixels);
	DisposeGWorld(offscreen);
	DisposeWindow(newWindow);
	
}

// probably going to need to do TPF - ticks per frame





void main2()
{
	Vector3 a = {1, 1, 1}, b = {2, 2, 2}, c;
	float size = 20;
	float xangle = 45, yangle = 60, zangle = 30;
//	Vector3 cube[8];
	Vector3 cube[8] = {
		{-size/2, -size/2,  size/2},
		{ size/2, -size/2,  size/2},
		{ size/2,  size/2,  size/2},
		{-size/2,  size/2,  size/2},
		{-size/2, -size/2, -size/2},
		{ size/2, -size/2, -size/2},
		{ size/2,  size/2, -size/2},
		{-size/2,  size/2, -size/2}
	};
	int i;
	Matrix3 rot = Matrix3::getRotationMatrix(xangle, yangle, zangle);
	Vector3 centre = {0, 0, 0};
	Vector3 rotated_cube[8];
	
	for (i = 0; i < 8; i++)
	{
//		cube[i] = {1.0f, 1.0f, 1.0f};
//		cout << i << endl;
//		cube[i].x = i;
//		cube[i].y = i;
//		cube[i].z = i;
		rotated_cube[i] = rot.preMultiply(cube[i]);
		rotated_cube[i] = rotated_cube[i].add(centre);
		rotated_cube[i] = rotated_cube[i].scale(40.0f / (-rotated_cube[i].z/2.0f + 40.0f));
		rotated_cube[i].x = rotated_cube[i].x + XRESOLUTION/2;
		rotated_cube[i].y = rotated_cube[i].y + YRESOLUTION/2;
	}
	


}

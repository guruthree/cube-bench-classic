// functions for main.cpp

#include <stdlib.h> // abs
#include <string.h> // strcpy & strcat

#include "config.h"
#include "cube.h"

// the functions from main in their own file to speed up
// recompiling on old hardware

#define M_PI 3.14159265359

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

// http://preserve.mactech.com/articles/develop/issue_26/minow.html
double MicrosecondToMillis(const UnsignedWide *time)
{
	// this needs to be a double - I tried just using lo and float but
	// I think it caused wrapping problems so yeah - double
	return ((((double)time->hi) * 4294967296.0) + time->lo) / 1000.0;
}

// display a help message
void writeHelp(short startX)
{
	short startY = 2;
	MoveTo(startX, startY += 13); // #1
	DrawString("\p>>> h: HELP <<<");
	MoveTo(startX, startY += 13);
	DrawString("\p`/esc/ret: quit");
	MoveTo(startX, startY += 13);
	DrawString("\pb: start benchmark");
	MoveTo(startX, startY += 13);
	DrawString("\pn/m: toggle rotate/move");
	MoveTo(startX, startY += 13); // #5
	DrawString("\pspc: toggle both");
	MoveTo(startX, startY += 13);
	DrawString("\pP: screenshot");
	MoveTo(startX, startY += 13);
	DrawString("\pT: save results to txt");
	MoveTo(startX, startY += 13);
	DrawString("\pr/R: reset/randomise cubes");
	MoveTo(startX, startY += 13);
	DrawString("\pv/c: invert/erase canvas");
	MoveTo(startX, startY += 13); // #10
	DrawString("\pf/-/+: fill/resize cube");
	MoveTo(startX, startY += 13);
	DrawString("\p1-0: toggle cube");
	MoveTo(startX, startY += 13);
	DrawString("\px: toggle bounce");
	MoveTo(startX, startY += 13);
	DrawString("\pjlikuo: rotation");
	MoveTo(startX, startY += 13);
	DrawString("\pJLIKUO: rotation speed");
	MoveTo(startX, startY += 13); // # 15
	DrawString("\padwsqe: move");
	MoveTo(startX, startY += 13);
	DrawString("\pADWSQE: move speed");
	// 16 lines total
}

// generate random float, inclusive
float rand(float mi, float ma)
{
	// Random() is -32,768 to 32,767, so abs to get 0-32,767
	// multiply by the range of value to keep precision
	// devide by original max to scale
	// add in mi to put in correct range
	return (abs(Random()) * (ma - mi)) / 32767.0 + mi;
}

// reset all of the cubes
void resetCubes(Cube *cubes[], Boolean *activeCubes, short xRes, short yRes)
{
	short i, j, c = 0;

	// cube 0
	if (activeCubes[c])
	{
		cubes[c]->reset();
		cubes[c]->updateSize(100);
	}
	c++; if (c >= NUM_CUBES) return;

	short offsets[2] = {-1, 1};

	// cubes 1-4
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 2; j++)
		{
			if (activeCubes[c])
			{
				cubes[c]->reset();
				cubes[c]->updateSize(xRes / 10);
				cubes[c]->translate(Vector3(offsets[i] * xRes / 4, offsets[j] * yRes / 4, 0));
				cubes[c]->dangle.x *= offsets[i];
				// cubes[c]->dangle.y *= offsets[j];
				cubes[c]->dangle.z *= offsets[j];
			}
			c++; if (c >= NUM_CUBES) return;
		}
	}

	// cubes 5-6
	for (i = 0; i < 2; i++)
	{
		if (activeCubes[c])
		{
			cubes[c]->reset();
			cubes[c]->updateSize(xRes / 20);
			cubes[c]->translate(Vector3(offsets[i] * 3 * xRes / 7, 0, -100));
			cubes[c]->velocity.x *= -1;
			cubes[c]->velocity.y *= -1;
			cubes[c]->velocity.z *= -1;
			cubes[c]->dangle.x *= offsets[i];
			cubes[c]->dangle.y *= -1;
			cubes[c]->dangle.z *= -1;
		}
		c++; if (c >= NUM_CUBES) return;
	}

	// cubes 7-9
	for (i = 0; i < 2; i++)
	{
		if (activeCubes[c])
		{
			cubes[c]->reset();
			cubes[c]->updateSize(xRes / 20);
			cubes[c]->translate(Vector3(0, offsets[i] * 3 * yRes / 7, -100));
			cubes[c]->velocity.x *= -1;
			cubes[c]->velocity.y *= -1;
			cubes[c]->velocity.z *= -1;
			cubes[c]->dangle.x *= -1;
			cubes[c]->dangle.y *= offsets[i];
			cubes[c]->dangle.z *= -1;
		}
		c++; if (c >= NUM_CUBES) return;
	}

	// cube 10
	if (activeCubes[c])
	{
		cubes[c]->reset();
		cubes[c]->updateSize(yRes * 1.5);
		cubes[c]->translate(Vector3(0, 0, -1000));
		cubes[c]->velocity.x = 0;
		cubes[c]->velocity.y = 0;
		cubes[c]->velocity.z = 0;
		cubes[c]->dangle.x *= 0.1;
		cubes[c]->dangle.y *= 0.1;
		cubes[c]->dangle.z *= 0.1;
	}
}

// randomise all of the cubes
void randomiseCubes(Cube *cubes[], Boolean *activeCubes, short xRes, short yRes)
{
	short i;
	for (i = 0; i < NUM_CUBES; i++)
	{
		if (activeCubes[i])
		{
			// reset cube
			cubes[i]->reset();
			// cube size (2 to 15)*10 (rand 0-13 + 2)
			cubes[i]->updateSize(rand(20, 150));
			// location from xRes, yRes, and depth of -1000 to 100
			cubes[i]->translate(Vector3(
				rand(-xRes / 2, xRes / 2),
				rand(-yRes / 2, yRes / 2),
				rand(-1000, 100)));
			// rotation 0 to pi/2
			cubes[i]->rotate(Vector3(
				rand(0, M_PI),
				rand(0, M_PI),
				rand(0, M_PI)));
			// velocity 0.2-1.8
			cubes[i]->velocity.x += rand(-1.8, 1.8);
			cubes[i]->velocity.y += rand(-1.2, 1.2);
			cubes[i]->velocity.z += rand(-0.7, 0.7);
			// angular velocity 0.001 to 0.05
			cubes[i]->dangle.x += rand(-0.04, 0.04);
			cubes[i]->dangle.y += rand(-0.04, 0.04);
			cubes[i]->dangle.z += rand(-0.04, 0.04);
		}
	}
}

// gather some info about the system
void getCPUandFPU(char FPUbuffer[], SysEnvRec *sys_info)
{
	long CPUtype, FPUtype;
	if (Gestalt(gestaltNativeCPUtype, &CPUtype) == noErr)
	{
		switch (CPUtype)
		{
		case gestaltCPU68000:
		case gestaltCPU68010:
		case gestaltCPU68020:
		case gestaltCPU68030:
		case gestaltCPU68040:
			strcpy(FPUbuffer, "68000 CPU");
			FPUbuffer[3] = CPUtype + '0';
			break;

		case gestaltCPU601:
		case gestaltCPU603:
		case gestaltCPU604:
			strcpy(FPUbuffer, "  600 CPU");
			FPUbuffer[4] = CPUtype - 0x100 + '0';
			break;

		default:
			strcpy(FPUbuffer, "   ?? CPU");
			break;
		}
	}
	else
	{
		// fall back to SysEnvRec
		if (sys_info->processor == envCPUUnknown)
		{
			strcpy(FPUbuffer, "  ERR CPU");
		}
		else
		{
			strcpy(FPUbuffer, "68000 CPU");
			FPUbuffer[3] = sys_info->processor + '0' - 1;
		}
	}

	if (Gestalt(gestaltFPUType, &FPUtype) == noErr)
	{
		switch (FPUtype)
		{
		case gestaltNoFPU:
			strcat(FPUbuffer, " / NO FPU");
			break;

		case gestalt68881:
			strcat(FPUbuffer, " / 68881 FPU");
			break;

		case gestalt68882:
			strcat(FPUbuffer, " / 68882 FPU");
			break;

		case gestalt68040FPU:
			strcat(FPUbuffer, " / 68040 FPU");
			break;

		default:
			strcat(FPUbuffer, " /	?? FPU");
			break;
		}
	}
	else
	{
		strcat(FPUbuffer, "CPU / ERR FPU");
	}
#if !mc68881
	// a 6XXXX FPU
	if (FPUbuffer[12] == '6')
		strcat(FPUbuffer, " (DISABLED)");
#endif
	CtoPstr(FPUbuffer); // display functions expect P strings
}

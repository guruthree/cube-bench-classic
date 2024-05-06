#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
float MicrosecondToFloatMillis(const UnsignedWide *time)
{
    // this needs to be a double, so we'll ignore the hi bits and just
    // kind of assume wrapping works OK as we only need time differences
    // return ((((float)time->hi) * 4294967296.0) + time->lo) / 1000.0;
    return (time->lo) / 1000.0;
}

// display the Ticks Per Frame (1 tick ~= 1/60 s)
void writeStats(char buffer[], unsigned char TPF, float frametime)
{
    sprintf(buffer, "TPF: %hu", TPF);
    CtoPstr(buffer);
    MoveTo(7, 15);
    DrawString((unsigned char *)buffer);

    // my SE doesn't seem to do %0.1f, so (short) it is
    sprintf(buffer, "FPS: %hu", (short)(1.0 / (frametime / 1000.0)));
    CtoPstr(buffer);
    MoveTo(7, 15 + 13);
    DrawString((unsigned char *)buffer);

    sprintf(buffer, "FT: %hu ms", (short)frametime);
    CtoPstr(buffer);
    MoveTo(7, 15 + 13 * 2);
    DrawString((unsigned char *)buffer);
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

// randomise all of the cubes
void randomiseCubes(Cube *cubes[], Boolean *activeCubes, short xRes, short yRes, short num_cubes)
{
    short i;
    for (i = 0; i < num_cubes; i++)
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
    if (noErr == Gestalt(gestaltNativeCPUtype, &CPUtype))
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

    if (noErr == Gestalt(gestaltFPUType, &FPUtype))
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

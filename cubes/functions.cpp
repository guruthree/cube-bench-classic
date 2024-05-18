// functions for main.cpp

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cube.h"

// the functions from main in their own file to speed up
// recompiling on old hardware

#define M_PI 3.14159265359

// keep a 1 minute (full benchmark) record of stats
#define LONG_STATS

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

// for storing stats while running
#define CONCAT(a, b) a##b
#define NEW_ARRAY(NAME, LEN)      \
    float CONCAT(NAME, LEN)[LEN]; \
    short CONCAT(NAME, LEN)##_at = -1;
#define INIT_ARRAY(NAME, LEN)                      \
    memset(CONCAT(NAME, LEN), LEN, sizeof(float)); \
    CONCAT(NAME, LEN)                              \
    ##_at = 0;
#define LOOP_ARRAY(NAME, LEN)          \
    if (CONCAT(NAME, LEN)##_at == LEN) \
    {                                  \
        CONCAT(NAME, LEN)              \
        ##_at = 0;                     \
    }
#define ADD2_ARRAY(NAME, LEN, VALUE) CONCAT(NAME, LEN)[CONCAT(NAME, LEN)##_at++] = VALUE;
#define MEAN(NAME, LEN) mean(CONCAT(NAME, LEN), LEN)
#define STD(NAME, MU, LEN) std(CONCAT(NAME, LEN), MU, LEN)

float mean(float *x, short len)
{
    short i;
    double sum = 0;
    for (i = 0; i < len; i++)
    {
        sum += x[i];
    }
    return sum / len;
}

float std(float *x, float mu, short len)
{
    short i;
    double sum = 0;
    for (i = 0; i < len; i++)
    {
        sum += pow(x[i] - mu, 2);
    }
    return sqrt(sum / len);
}

#define LAST_LEN 30
#define LONG_LAST_LEN 3600

NEW_ARRAY(last, LAST_LEN)
NEW_ARRAY(lastfps, LAST_LEN)
#ifdef LONG_STATS
NEW_ARRAY(last, LONG_LAST_LEN)
#endif

// need a way to show decimals on the SE, which doesn't agree with %0.1f
void tenthsPlace(float in, short &f, short &tenths)
{
    // return the value in the tens place
    f = (short)floor(in);
    tenths = (short)(10 * (in - f));
}

// display the Ticks Per Frame (1 tick ~= 1/60 s)
void writeStats(char buffer[], unsigned char TPF, float frametime)
{
    short i, f1, t1, f2, t2;
    if (last30_at == -1) // this line will error if the preprocessor struggles
    {
        INIT_ARRAY(last, LAST_LEN)
        INIT_ARRAY(lastfps, LAST_LEN)
#ifdef LONG_STATS
        INIT_ARRAY(last, LONG_LAST_LEN)
#endif
    }

    sprintf(buffer, "TPF: %hu", TPF);
    CtoPstr(buffer);
    MoveTo(7, 15);
    DrawString((unsigned char *)buffer);

    // store stats
    float frametime2 = (1.0 / (frametime / 1000.0));
    ADD2_ARRAY(last, LAST_LEN, frametime)
    ADD2_ARRAY(lastfps, LAST_LEN, frametime2)
#ifdef LONG_STATS
    ADD2_ARRAY(last, LONG_LAST_LEN, frametime)
#endif

    // mean & std fps
    float mu = MEAN(lastfps, LAST_LEN);
    float sig = STD(lastfps, mu, LAST_LEN);

    // my SE doesn't seem to do %0.1f, so (short) it is
    // sprintf(buffer, "FPS: %0.1f +- %0.1f", mu, sig);
    tenthsPlace(mu, f1, t1);
    tenthsPlace(sig, f2, t2);
    sprintf(buffer, "FPS: %hu.%hu +- %hu.%hu", f1, t1, f2, t2);
    CtoPstr(buffer);
    MoveTo(7, 15 + 13);
    DrawString((unsigned char *)buffer);

    // mean & std frametime
    mu = MEAN(last, LAST_LEN);
    sig = STD(last, mu, LAST_LEN);

    // sprintf(buffer, "FT: %0.1f +- %0.1f ms", mu, sig);
    tenthsPlace(mu, f1, t1);
    tenthsPlace(sig, f2, t2);
    sprintf(buffer, "FT: %hu.%hu +- %hu.%hu ms", f1, t1, f2, t2);
    CtoPstr(buffer);
    MoveTo(7, 15 + 13 * 2);
    DrawString((unsigned char *)buffer);

    LOOP_ARRAY(last, LAST_LEN)
    LOOP_ARRAY(lastfps, LAST_LEN)
#ifdef LONG_STATS
    LOOP_ARRAY(last, LONG_LAST_LEN)
#endif
}

// display a help message
void writeHelp(short startX)
{
    short startY = 2;
    MoveTo(startX, startY += 13); // #1
    DrawString("\p>>> h: HELP <<<");
    MoveTo(startX, startY += 13); // #2
    DrawString("\p`/esc/ret: quit");
    MoveTo(startX, startY += 13); // #3
    DrawString("\pn/m/spc: toggle rotate/move");
    MoveTo(startX, startY += 13); // #4
    DrawString("\pspc: toggle both");
    MoveTo(startX, startY += 13); // #5
    DrawString("\pP: screenshot");
    MoveTo(startX, startY += 13); // #6
    DrawString("\pr/R: reset/randomise cubes");
    MoveTo(startX, startY += 13); // #7
    DrawString("\pv/c: invert/erase canvas");
    MoveTo(startX, startY += 13); // #8
    DrawString("\pf/-/+: fill/resize cube");
    MoveTo(startX, startY += 13); // #9
    DrawString("\p1-0: toggle cube");
    MoveTo(startX, startY += 13); // #10
    DrawString("\pjlikuo: rotation");
    MoveTo(startX, startY += 13); // #11
    DrawString("\pJLIKUO: rotation speed");
    MoveTo(startX, startY += 13); // #12
    DrawString("\padwsqe: move");
    MoveTo(startX, startY += 13); // #13
    DrawString("\pADWSQE: move speed");
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
void resetCubes(Cube *cubes[], Boolean *activeCubes, short xRes, short yRes, short num_cubes)
{
    short i, j, c = 0;

    // cube 0
    if (activeCubes[c])
    {
        cubes[c]->reset();
        cubes[c]->updateSize(100);
    }
    c++;

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
            c++;
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
        c++;
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
        c++;
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

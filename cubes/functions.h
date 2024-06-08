// header for functions for main.cpp

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cube.h"

// the functions from main in their own file to speed up
// recompiling on old hardware

#pragma once

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// boilerplate application initialisation
void InitToolbox();

// http://preserve.mactech.com/articles/develop/issue_26/minow.html
float MicrosecondToFloatMillis(const UnsignedWide *time);

// display a help message
void writeHelp(short startX);

// generate random float, inclusive
float rand(float mi, float ma);

// reset all of the cubes
void resetCubes(Cube *cubes[], Boolean *activeCubes, short xRes, short yRes);

// randomise all of the cubes
void randomiseCubes(Cube *cubes[], Boolean *activeCubes, short xRes, short yRes);

// gather some info about the system
void getCPUandFPU(char FPUbuffer[], SysEnvRec *sys_info);

#endif

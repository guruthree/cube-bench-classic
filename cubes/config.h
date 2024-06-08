#pragma once

#ifndef CONFIG_H
#define CONFIG_H

// configuration options for cube bench classic

// how much smaller than the resolution to make the window
#define SHRINK 40

// use back buffer to render to, at the cost of some performance
#define USEOFFSCREEN

// clear the entire/canvase back buffer
// (instead of just the cube by writing over it)
// #define ERASECANVAS

// how many cubes we could be rendering
#define NUM_CUBES 10

// offset from the right for help message
#define HELP_OFFSET 202

// moving average of 30 frames worth to calcualate on screen stats
#define SHORT_STATS 30

// number of frames to make up the benchmark
// keep a record of frame times for the entire benchmark
// at 60 fps this would be a 30 second benchmark
#define LONG_STATS 1800

#endif

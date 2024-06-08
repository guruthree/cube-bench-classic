// things for showing fps and benchmarking stats

// needed for GWorldPtr
#include <QDOffscreen.h>

#include <math.h>
#include <stdlib.h> // malloc & free
#include <stdio.h>	// sprintf
#include <string.h> // memset

#include "config.h"
#include "statistics.h"
#include "filefunctions.h"
#include "version.h"

// initialise to the desired length
Statistics::Statistics(short myLen, Boolean doPlete)
{
	mu = 0;
	sigma = 0;
	len = myLen;
	values = (float *)calloc(len, sizeof(float));
	at = 0;
	reset();
	doComplete = doPlete;
}

// we need to free the malloc
Statistics::~Statistics()
{
	if (values == NULL)
	{
		return;
	}
	free(values);
}

// clear all recorded values
void Statistics::reset()
{
	memset(values, len, sizeof(float));
	completed = false;
	at = 0;
}

// update the next value in the array
void Statistics::addValue(float newValue)
{
	if (completed)
		return;
	values[at++] = newValue;
	if (at == len)
	{
		if (doComplete)
		{
			completed = true;
			mean();
			std();
		}
		at = 0;
	}
}

// calculate the mean (average) of recorded values
float Statistics::mean()
{
	// if (completed)
	// {
	// 	// no point in updating if we're done
	// 	return;
	// }
	short i;
	double sum = 0;
	for (i = 0; i < len; i++)
	{
		sum += values[i];
	}
	mu = sum / len;
	return mu;
}

// calcluate the standard deviation of recorded values, assuming mu is up to date
float Statistics::std()
{
	// if (completed)
	// {
	// 	// no point in updating if we're done
	// 	return;
	// }
	short i;
	double sum = 0;
	for (i = 0; i < len; i++)
	{
		sum += pow(values[i] - mu, 2);
	}
	sigma = sqrt(sum / len);
	return sigma;
}

// write the stat on screen
void Statistics::write(short startY, const char label[])
{
	char buffer[40];

	// nothing to show while we're running (the calculating _chugs_)
	if (doComplete && !completed)
	{
		sprintf(buffer, "%s: %hu%% done", label, (100 * at) / len);
		CtoPstr(buffer);
		MoveTo(7, startY);
		DrawString((unsigned char *)buffer);
		return;
	}

	short f1, t1, f2, t2;

	// run calculations
	if (!completed)
	{
		mean();
		std();
	}

	// my SE doesn't seem to do %0.1f, so (short) it is
	// sprintf(buffer, "FPS: %0.1f +- %0.1f", mu, sig);
	tenthsPlace(mu, f1, t1);
	tenthsPlace(sigma, f2, t2);
	sprintf(buffer, "%s: %hu.%hu +- %hu.%hu", label, f1, t1, f2, t2);
	CtoPstr(buffer);
	MoveTo(7, startY);
	DrawString((unsigned char *)buffer);
}

// write the stats out to a file
OSErr Statistics::writeToFile(const unsigned char defaultName[], GWorldPtr offScreen,
							  char FPUbuffer[], Boolean *activeCubes, Boolean wireFrame)
{
	// note file code here based on code from takeScreenshot()
	// the extra arguments are for writing out system and config info, etc

	short i; // for looping
	OSErr err = noErr;
	short fid;
	char buffer[40]; // for writing out
	short f1, t1;

	// Open a StandardPutFile for saving (via saveDialog)
	StandardFileReply myReply;
	err = saveDialog(&myReply, defaultName);
	if (err != noErr)
	{
		return err;
	}

	// path to the file
	FSSpec outFile = myReply.sfFile;

	// create empty file with data and resource forks?
	// 'ttxt' marks TeachText/SimpleText as the creator (via Macintosh Game Programming Techniques)
	// 'TEXT' should be the 4 character resource fork id/code (via the Macintosh Toolbox Essentials)
	err = FSpCreate(&outFile, 'ttxt', 'TEXT', myReply.sfScript);
	if (err != noErr)
	{
		SysBeep(1);
		return err;
	}

	err = FSpOpenDF(&outFile, fsWrPerm, &fid);
	if (err != noErr)
	{
		SysBeep(1);
		return err;
	}

	// write the benchmark results
	writeString(fid, "Cube Bench Classic Benchmark results\r");

	// information about the version
	writeString(fid, "Version, ");
	writeString(fid, CUBE_VERSION);
	write_char(fid, '\r');

	// screen resolution
	sprintf(buffer, "Resolution, %hu x %hu\r",
			screenBits.bounds.right - screenBits.bounds.left,
			screenBits.bounds.bottom - screenBits.bounds.top);
	writeString(fid, buffer);

	// colour depth (see screenshot.cpp)
	PixMapHandle pixmap = GetGWorldPixMap(offScreen);
	sprintf(buffer, "Colour depth, %hu bpp\r", (**(pixmap)).pixelSize);
	writeString(fid, buffer);

	// window size
	sprintf(buffer, "Window size, %hu x %hu\r",
			(**pixmap).bounds.right - (**pixmap).bounds.left,
			(**pixmap).bounds.bottom - (**pixmap).bounds.top);
	writeString(fid, buffer);

	// cpu & fpu
	writeString(fid, "System specs, ");
	writeString(fid, FPUbuffer);
	write_char(fid, '\r');

	// filled or solid
	if (wireFrame)
	{
		writeString(fid, "Cube type, wireframe\r");
	}
	else
	{
		writeString(fid, "Cube type, solid\r");
	}

	// number of cubes (& which cubes)
	for (i = 0; i < NUM_CUBES; i++)
	{
		if (activeCubes[i])
		{
			sprintf(buffer, "Cube %hu, active\r", i);
		}
		else
		{
			sprintf(buffer, "Cube %hu, inactive\r", i);
		}
		writeString(fid, buffer);
	}

	if (completed)
	{
		// fps summary
		// TODO

		// frame time summary
		// TODO

		// write out all individual frame times
		writeString(fid, "Frame Number, Frame Time\r");
		for (i = 0; i < len; i++)
		{
			tenthsPlace(values[i], f1, t1);
			sprintf(buffer, "%hu, %hu.%hu\r", i, f1, t1);
			writeString(fid, buffer);
		}
		write_char(fid, '\r');
	}

	err = FSClose(fid);
	if (err != noErr)
	{
		SysBeep(1);
		return err;
	}

	return noErr;
}

// need a way to show decimals on the SE, which doesn't agree with %0.1f
void tenthsPlace(float in, short &f, short &tenths)
{
	// return the value in the tens place
	f = (short)floor(in);
	tenths = (short)(10 * (in - f));
}

// display the Ticks Per Frame (1 tick ~= 1/60 s)
void writeTPF(unsigned char TPF)
{
	char buffer[20];
	sprintf(buffer, "TPF: %hu", TPF);
	CtoPstr(buffer);
	MoveTo(7, 15);
	DrawString((unsigned char *)buffer);
}

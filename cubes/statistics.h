// header for functions for statistics.cpp

#pragma once

#ifndef STATISTICS_H
#define STATISTICS_H

// for storing stats while running
class Statistics
{
private:
	float mu;
	float sigma;

	short len;
	float *values;
	short at;
	Boolean doComplete;

public:
	// is this a completed benchmark record
	Boolean completed;

	// initialise to the desired length
	Statistics(short myLen, Boolean doPlete);
	// we need to free the malloc
	~Statistics();

	// clear all recorded values
	void reset();

	// update the next value in the array
	void addValue(float newValue);

	// calculate the mean (average) of recorded values
	float mean();
	// calcluate the standard deviation of recorded values, assuming mu is up to date
	float std();

	// write the stat on screen
	void write(short startY, const char label[]);

	// write the stats out to a file
	OSErr writeToFile(const unsigned char defaultName[], GWorldPtr offScreen,
					  char FPUbuffer[], Boolean *activeCubes, Boolean wireFrame,
					  char TIMEbuffer[]);
};

// need a way to show decimals on the SE, which doesn't agree with %0.1f
void tenthsPlace(float in, short &f, short &tenths);

// display the Ticks Per Frame (1 tick ~= 1/60 s)
void writeTPF(unsigned char TPF);

#endif

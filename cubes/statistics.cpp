// things for showing fps and benchmarking stats

#include <math.h>
#include <stdlib.h> // malloc & free
#include <stdio.h>	// sprintf
#include <string.h> // memset

#include "statistics.h"

// initialise to the desired length
Statistics::Statistics(short myLen)
{
	mu = 0;
	sigma = 0;
	len = myLen;
	values = (float *)calloc(len, sizeof(float));
	at = 0;
	reset();
	completed = 0;
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
}

// update the next value in the array
void Statistics::addValue(float newValue)
{
	values[at++] = newValue;
	if (at == len)
	{
		at = 0;
	}
}

// calculate the mean (average) of recorded values
float Statistics::mean()
{
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
	short f1, t1, f2, t2;
	
	// run calculations
	mean();
	std();

	// my SE doesn't seem to do %0.1f, so (short) it is
	// sprintf(buffer, "FPS: %0.1f +- %0.1f", mu, sig);
	tenthsPlace(mu, f1, t1);
	tenthsPlace(sigma, f2, t2);
	sprintf(buffer, "%s: %hu.%hu +- %hu.%hu", label, f1, t1, f2, t2);
	CtoPstr(buffer);
	MoveTo(7, startY);
	DrawString((unsigned char *)buffer);
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

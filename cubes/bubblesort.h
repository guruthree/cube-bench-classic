// From ChatGPT
// "How can I sort two arrays simultaneously, ordered by the values
// in one using ANSI C?"

#pragma once

#ifndef BUBBLESORT
#define BUBBLESORT

// Function to swap elements in both arrays
void bSswap(float *a, float *b, int *c, int *d);

// Function to perform bubble sort
void bubbleSort(float keys[], int values[], int n);
#endif
// From ChatGPT
// "How can I sort two arrays simultaneously, ordered by the values
// in one using ANSI C?"


#ifndef BUBBLESORT
#define BUBBLESORT

// Function to swap elements in both arrays
void bSswap(float *a, float *b, int *c, int *d) {
    float temp = *a;
    *a = *b;
    *b = temp;
    
    int tmp = *c;
    *c = *d;
    *d = tmp;
}

// Function to perform bubble sort
void bubbleSort(float keys[], int values[], int n) {
    int i, j;
    for (i = 0; i < n-1; i++) {
        for (j = 0; j < n-i-1; j++) {
            if (keys[j] > keys[j+1]) {
                bSswap(&keys[j], &keys[j+1], &values[j], &values[j+1]);
            }
        }
    }
}

// bubblesort(averageDepths, indexes)

#endif
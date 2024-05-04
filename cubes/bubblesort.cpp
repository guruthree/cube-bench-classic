// From ChatGPT
// "How can I sort two arrays simultaneously, ordered by the values
// in one using ANSI C?"

// Function to swap elements in both arrays
void bSswap(float *a, float *b, short *c, short *d)
{
	float temp = *a;
	*a = *b;
	*b = temp;

	short tmp = *c;
	*c = *d;
	*d = tmp;
}

// Function to perform bubble sort, soring  values array by contents of keys
void bubbleSort(float keys[], short values[], short n)
{
	short i, j;
	for (i = 0; i < n - 1; i++)
	{
		for (j = 0; j < n - i - 1; j++)
		{
			if (keys[j] < keys[j + 1])
			{
				bSswap(&keys[j], &keys[j + 1], &values[j], &values[j + 1]);
			}
		}
	}
}

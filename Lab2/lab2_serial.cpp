#include <stdio.h>
#include <iostream>
#include <ctime>

using namespace std;

//Swap function
void swap(int* xp, int* yp)
{
	int temp = *xp;
	*xp = *yp;
	*yp = temp;
}

void selectionSort(int arr[], int n)
{
	int i, j, min_idx;
	for (i = 0; i < n - 1; i++)
	{

		// Find the minimum element in
		// unsorted array
		min_idx = i;
		for (j = i + 1; j < n; j++)
			if (arr[j] < arr[min_idx])
				min_idx = j;

		// Swap the found minimum element
		// with the first element
		if (min_idx != i)
			swap(&arr[min_idx], &arr[i]);
	}
}


// Driver program to test above functions
int main(int argc, char** argv)
{
	int n;
	int* data = NULL;
	FILE* file = NULL;
	if (argc != 3) {
		fprintf(stderr, "Usage: <in_file> <out_file>\n", argv[0]);
		exit(1);
	}

	file = fopen(argv[1], "r");
	fscanf(file, "%d", &n);

	// read data from file
	data = (int*)malloc(n * sizeof(int));
	for (int i = 0; i < n; i++)
		fscanf(file, "%d", &(data[i]));
	fclose(file);

	double start_time = clock();
	selectionSort(data, n);
	double end_time = clock();
	//write data to file
	file = fopen(argv[2], "w");
	fprintf(file, "%d\n", n);  
	for (int i = 0; i < n; i++)
		fprintf(file, "%d\n", data[i]);
	fclose(file);
	free(data);
	printf("selectionSort serial: %f secs\n", (end_time - start_time) / 1000);

	return 0;
}


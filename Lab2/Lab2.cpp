#include <stdio.h>
#include <iostream>
#include <mpi.h>

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

	// One by one move boundary of
	// unsorted subarray
	for (i = 0; i < n - 1; i++)
	{
		min_idx = i;
		for (j = i + 1; j < n; j++)
			if (arr[j] < arr[min_idx])
				min_idx = j;
		if (min_idx != i)
			swap(&arr[min_idx], &arr[i]);
	}
}

int* merge(int* arr1, int size1, int* arr2, int size2)
{
	int* result = (int*)malloc((size1 + size2) * sizeof(int));
	int i = 0;
	int j = 0;
	for (int k = 0; k < size1 + size2; k++) {
		if (i >= size1) {
			result[k] = arr2[j];
			j++;
		}
		else if (j >= size2) {
			result[k] = arr1[i];
			i++;
		}
		else if (arr1[i] < arr2[j]) { // indices in bounds as i < n1 && j < n2
			result[k] = arr1[i];
			i++;
		}
		else { // v2[j] <= v1[i]
			result[k] = arr2[j];
			j++;
		}
	}
	return result;
}

int main(int argc, char** argv)
{
	int n;
	int* data = NULL;
	int c, s;
	int* subarr1;
	int o;
	int* subarr2;
	int step;
	int size, rank;
	MPI_Status status;
	double elapsed_time;
	FILE* file = NULL;
	int i;

	if (argc != 3) {
		fprintf(stderr, "Usage: mpirun -np <num_procs> %s <in_file> <out_file>\n", argv[0]);
		exit(1);
	}

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0) {
		// read size of data
		file = fopen(argv[1], "r");
		fscanf(file, "%d", &n);
		// compute chunk size
		c = n / size; if (n % size) c++;
		// read data from file
		data = (int*)malloc(size * c * sizeof(int));
		for (i = 0; i < n; i++)
			fscanf(file, "%d", &(data[i]));
		fclose(file);
		// pad data with 0 -- doesn't matter
		for (i = n; i < size * c; i++)
			data[i] = 0;
	}

	// start the timer
	MPI_Barrier(MPI_COMM_WORLD);
	elapsed_time = -MPI_Wtime();

	// broadcast size
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// compute chunk size
	c = n / size; if (n % size) c++;

	// scatter data
	subarr1 = (int*)malloc(c * sizeof(int));
	MPI_Scatter(data, c, MPI_INT, subarr1, c, MPI_INT, 0, MPI_COMM_WORLD);
	free(data);
	data = NULL;

	// compute size of own chunk and sort it
	s = (n >= c * (rank + 1)) ? c : n - c * rank;
	selectionSort(subarr1, s);

	// up to log_2 p merge steps
	for (step = 1; step < size; step = 2 * step) {
		if (rank % (2 * step) != 0) {
			// id is no multiple of 2*step: send chunk to id-step and exit loop
			MPI_Send(subarr1, s, MPI_INT, rank - step, 0, MPI_COMM_WORLD);
			break;
		}
		// id is multiple of 2*step: merge in chunk from id+step (if it exists)
		if (rank + step < size) {
			// compute size of chunk to be received
			o = (n >= c * (rank + 2 * step)) ? c * step : n - c * (rank + step);
			// receive other chunk
			subarr2 = (int*)malloc(o * sizeof(int));
			MPI_Recv(subarr2, o, MPI_INT, rank + step, 0, MPI_COMM_WORLD, &status);
			// merge and free memory
			data = merge(subarr1, s, subarr2, o);
			free(subarr1);
			free(subarr2);
			subarr1 = data;
			s = s + o;
		}
	}

	// stop the timer
	elapsed_time += MPI_Wtime();

	// write sorted data to out file and print out timer
	if (rank == 0) {
		file = fopen(argv[2], "w");
		fprintf(file, "%d\n", s);   // assert (s == n)
		for (i = 0; i < s; i++)
			fprintf(file, "%d\n", subarr1[i]);
		fclose(file);
		printf("selectionSort %d ints on %d procs: %f secs\n", n, size, elapsed_time);
		// printf("%d %2d %f\n", n, p, elapsed_time);
	}

	MPI_Finalize();
	return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define MASTER_RANK 0
void swap(int *a, int *b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}
int partition(int arr[], int low, int high)
{
	int pivot = arr[high];
	int i = (low - 1);
	for (int j = low; j <= high - 1; j++)
	{
		if (arr[j] < pivot)
		{
			i++;
			swap(&arr[i], &arr[j]);
		}
	}
	swap(&arr[i + 1], &arr[high]);
	return (i + 1);
}
void quicksort(int arr[], int low, int high)
{

	if (low < high)
	{
		int pi = partition(arr, low, high);
		quicksort(arr, low, pi - 1);
		quicksort(arr, pi + 1, high);
	}
}
void merge(int arr[], int left[], int right[], int n1, int n2)
{
	int i = 0, j = 0, k = 0;
	while (i < n1 && j < n2)
	{
		if (left[i] <= right[j])
			arr[k++] = left[i++];

		else
			arr[k++] = right[j++];
	}
	while (i < n1)
		arr[k++] = left[i++];

	while (j < n2)
		arr[k++] = right[j++];
}
int main(int argc, char **argv)
{
	int rank, size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int n = 12;											  // Size of the array
	int arr[] = {12, 11, 13, 5, 6, 7, 9, 8, 10, 1, 2, 4}; // Array to be sorted

	int *localArr = (int *)malloc(sizeof(int) * n / size);
	MPI_Scatter(arr, n / size, MPI_INT, localArr, n / size, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	int localSize = n / size;
	quicksort(localArr, 0, localSize - 1);
	int step = 1;

	while (step < size)
	{
		if (rank % (2 * step) == 0)
		{
			if (rank + step < size)
			{
				int recvSize;
				MPI_Recv(&recvSize, 1, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

				int *recvArr = (int *)malloc(sizeof(int) * recvSize);
				MPI_Recv(recvArr, recvSize, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

				int *mergedArr = (int *)malloc(sizeof(int) * (localSize + recvSize));

				merge(mergedArr, localArr, recvArr, localSize, recvSize);
				localArr = mergedArr;
				localSize += recvSize;
			}
		}
		else
		{
			int sendRank = rank - step;
			MPI_Send(&localSize, 1, MPI_INT, sendRank, 0, MPI_COMM_WORLD);
			MPI_Send(localArr, localSize, MPI_INT, sendRank, 0, MPI_COMM_WORLD);
			break;
		}
		step *= 2;
	}
	if (rank == MASTER_RANK)
	{
		printf("Sorted array: ");
		for (int i = 0; i < localSize; i++)
			printf("%d ", localArr[i]);

		printf("\n");
	}
	MPI_Finalize();
	return 0;
}
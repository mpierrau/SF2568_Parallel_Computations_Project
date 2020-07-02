#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>
#define LOW 0
#define HIGH 1

double *local_list, *temp_list, *merge_list;
double startT, stopT;

void mergeLow(int list_size, double *list1, double *list2)
{
	int i;
	int index1 = 0;
	int index2 = 0;
	merge_list = (double*) malloc(list_size* sizeof(double));
	for (i = 0; i < list_size; i++)
		if (list1[index1] <= list2[index2])
		{
			merge_list[i] = list1[index1];
			index1++;
		}
	else
	{
		merge_list[i] = list2[index2];
		index2++;
	}

	for (i = 0; i < list_size; i++)
		list1[i] = merge_list[i];
	free(merge_list);
}

void mergeHigh(int list_size, double *list1, double *list2)
{
	int i;
	int index1 = list_size - 1;
	int index2 = list_size - 1;
	merge_list = (double*) malloc(list_size* sizeof(double));
	for (i = list_size - 1; i >= 0; i--)
		if (list1[index1] >= list2[index2])
		{
			merge_list[i] = list1[index1];
			index1--;
		}
	else
	{
		merge_list[i] = list2[index2];
		index2--;
	}

	for (i = 0; i < list_size; i++)
		list1[i] = merge_list[i];
	free(merge_list);
}

void mergeSplit(int list_size, double *local_list, int which_keys, int partner, MPI_Comm comm)
{
	MPI_Status status;
	temp_list = (double*) malloc(list_size* sizeof(double));
	MPI_Sendrecv(local_list, list_size, MPI_DOUBLE, partner, 0, temp_list, list_size, MPI_DOUBLE, partner, 0, comm, &status);
	if (which_keys == HIGH)
		mergeHigh(list_size, local_list, temp_list);
	else
		mergeLow(list_size, local_list, temp_list);
	free(temp_list);
}

int compareDouble(const void *a, const void *b)
{
	if (*(double*) a > *(double*) b)
		return 1;
	else if (*(double*) a<*(double*) b)
		return -1;
	else
		return 0;
}



void bitonicsort_increase(int list_size, double *local_list, int processorSize, MPI_Comm comm)
{
	unsigned eor_bit;
	int processorDimension, stage, partner, my_rank;
	MPI_Comm_rank(comm, &my_rank);

	processorDimension = log2(processorSize);
	eor_bit = 1 << (processorDimension - 1);
	for (stage = 0; stage < processorDimension; stage++)
	{
		partner = my_rank ^ eor_bit;
		if (my_rank < partner)
			mergeSplit(list_size, local_list, LOW, partner, comm);
		else
			mergeSplit(list_size, local_list, HIGH, partner, comm);
		eor_bit = eor_bit >> 1;
	}
}

void bitonicsort_decrease(int list_size, double *local_list, int processorSize, MPI_Comm comm)
{
	unsigned eor_bit;
	int processorDimension, stage, partner, my_rank;
	MPI_Comm_rank(comm, &my_rank);

	processorDimension = log2(processorSize);
	eor_bit = 1 << (processorDimension - 1);
	for (stage = 0; stage < processorDimension; stage++)
	{
		partner = my_rank ^ eor_bit;
		if (my_rank > partner)
			mergeSplit(list_size, local_list, LOW, partner, comm);
		else
			mergeSplit(list_size, local_list, HIGH, partner, comm);
		eor_bit = eor_bit >> 1;
	}
}

int main(int argc, char *argv[])
{
	int list_size, n, i, processorSize, my_rank, p;
	unsigned andBit;
	int debug = 0;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	n = atoi(argv[1]);

	list_size = n / p;
	local_list = (double*) malloc(list_size* sizeof(double));

	srand(time(NULL) + my_rank);
	for (i = 0; i < list_size; i++)
	{
		local_list[i] = ((double) random()) / (RAND_MAX);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	if (debug)
	{
		printf("Processor %d:", my_rank);
		for (int i = 0; i < list_size; i++)
			printf("%f ", local_list[i]);
		printf("\n");
	}

	startT = MPI_Wtime();

	qsort(local_list, list_size, sizeof(double), compareDouble);

	for (processorSize = 2, andBit = 2; processorSize <= p; processorSize = processorSize *2, andBit = andBit << 1)
		if ((my_rank & andBit) == 0)
			bitonicsort_increase(list_size, local_list, processorSize, MPI_COMM_WORLD);
		else
			bitonicsort_decrease(list_size, local_list, processorSize, MPI_COMM_WORLD);
	// MPI_Barrier(MPI_COMM_WORLD);
	if (my_rank == 0)
	{
		stopT = MPI_Wtime();
	}
	MPI_Finalize();
	if (debug)
	{
		double *Print;
		Print = (double*) malloc(n* sizeof(double));
		MPI_Gather(local_list, list_size, MPI_DOUBLE, Print, list_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		if (my_rank == 0)
		{
			printf("N: %d\n", n);
			printf("P: %d\n", p);
			printf("Time(sec): %f\n", stopT - startT);

			printf("Final Sorted List: ");

			for (int i = 0; i < n; i++)
				printf("%.5f ", Print[i]);
			printf("\n");
		}
	}
	const char * filename = "Bitonic_Parallel_RunningTime.txt";
	if(my_rank==0)
	{
    FILE * f;
    f = fopen(filename, "a");
    fprintf(f, "%d %d %f \n", n, p, stopT - startT);

	free(local_list);}
}
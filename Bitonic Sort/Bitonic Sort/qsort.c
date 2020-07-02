#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

void printArray(double arr[], int size) {

  for (int i = 0; i < size; i++) {
    printf("%f ", arr[i]);
  }
  printf("\n");
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

int main(int argc, char *argv[])
{
    clock_t start, end;

    bool debug = atoi(argv[3]);
    int N1 = atoi(argv[1]);
    int N2 = atoi(argv[2]);
    int N = pow(2,N1);

    FILE *f;
    const char * filename = "serialRunningTimesQsort.txt";
    f = fopen(filename,"w");

    long double cpu_time_used;
    int its = N2 - N1 + 1;
    printf("No. its: %d\n", its);

    for(int k = 0; k < its; k++)
    {
        N = N*2;
        srand((unsigned) 1);

        double * arr = (double *) malloc(N* sizeof(double));
        for (int i = 0; i < N; i++)
        {
            arr[i] = ((double) rand()) / (RAND_MAX);
        }
        
        if (debug)
        {
            printf("Original array:");
            printArray(arr,N);
        }

        start = clock();
        qsort(arr,N,sizeof(double),compareDouble);
        end = clock();

        if (debug) {
            printf("Sorted array:");
            printArray(arr,N);
        }

        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

        printf("Number of elements: %d\n",N);
        printf("Time taken: %Lf seconds.\n", cpu_time_used);
        fprintf(f,"%d \t %Lf\n",N,cpu_time_used);
        }

        fclose(f);

    return 0;
}
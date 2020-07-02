#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

#define SIZE 10
double startT, stopT;
/**** Function Declaration Section ****/

static int intcompare(const void *i, const void *j)
{
  if ((*(int *)i) > (*(int *)j))
    return (1);
  if ((*(int *)i) < (*(int *)j))
    return (-1);
  return (0);
}



int main (int argc, char *argv[])
{
  /* Variable Declarations */

  int 	     Numprocs,MyRank, Root = 0;
  int 	     i,j,k, NoofElements, NoofElements_Bloc,
				  NoElementsToSort;
  int 	     count, temp;
  int 	     *Input, *InputData;
  int 	     *Splitter, *AllSplitter;
  int 	     *Buckets, *BucketBuffer, *LocalBucket;
  int 	     *OutputBuffer, *Output;
  FILE 	     *InputFile, *fp;
  MPI_Status  status; 
  
  /**** Initialising ****/
  
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &Numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &MyRank);

  if(argc != 2) {
      if(MyRank ==0) printf(" Usage : run size\n");
	 MPI_Finalize();
	 exit(0);
	}

  /**** Reading Input ****/
  
  if (MyRank == Root){

    NoofElements = atoi(argv[1]);
    Input = (int *) malloc (NoofElements*sizeof(int));
	 if(Input == NULL) {
		printf("Error : Can not allocate memory \n");
    }

  /* Initialise random number generator  */ 
  // printf ( "Input Array for Sorting \n\n ");
    srand48((unsigned int)NoofElements);
	 for(i=0; i< NoofElements; i++) {
       Input[i] = rand();
      //  printf ("%d   ",Input[i]);
    }
  }
  // printf ( "\n\n ");

  /**** Sending Data ****/
  MPI_Bcast (&NoofElements, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if(( NoofElements % Numprocs) != 0){
	    if(MyRank == Root)
		printf("Number of Elements are not divisible by Numprocs \n");
            MPI_Finalize();
	    exit(0);
  }

  NoofElements_Bloc = NoofElements / Numprocs;
  InputData = (int *) malloc (NoofElements_Bloc * sizeof (int));
  startT = MPI_Wtime();
  MPI_Scatter(Input, NoofElements_Bloc, MPI_INT, InputData, 
				  NoofElements_Bloc, MPI_INT, Root, MPI_COMM_WORLD);

  /**** Sorting Locally ****/
  qsort ((char *) InputData, NoofElements_Bloc, sizeof(int), intcompare);

  /**** Choosing Local Splitters ****/
  Splitter = (int *) malloc (sizeof (int) * (Numprocs-1));
  for (i=0; i< (Numprocs-1); i++){
        Splitter[i] = InputData[NoofElements/(Numprocs*Numprocs) * (i+1)];
  } 

  /**** Gathering Local Splitters at Root ****/
  AllSplitter = (int *) malloc (sizeof (int) * Numprocs * (Numprocs-1));
  MPI_Gather (Splitter, Numprocs-1, MPI_INT, AllSplitter, Numprocs-1, 
				  MPI_INT, Root, MPI_COMM_WORLD);

  /**** Choosing Global Splitters ****/
  if (MyRank == Root){
    qsort ((char *) AllSplitter, Numprocs*(Numprocs-1), sizeof(int), intcompare);

    for (i=0; i<Numprocs-1; i++)
      Splitter[i] = AllSplitter[(Numprocs-1)*(i+1)];
  }
  
  /**** Broadcasting Global Splitters ****/
  MPI_Bcast (Splitter, Numprocs-1, MPI_INT, 0, MPI_COMM_WORLD);

  /**** Creating Numprocs Buckets locally ****/
  Buckets = (int *) malloc (sizeof (int) * (NoofElements + Numprocs));
  
  j = 0;
  k = 1;

  for (i=0; i<NoofElements_Bloc; i++){
    if(j < (Numprocs-1)){
       if (InputData[i] < Splitter[j]) 
			 Buckets[((NoofElements_Bloc + 1) * j) + k++] = InputData[i]; 
       else{
	       Buckets[(NoofElements_Bloc + 1) * j] = k-1;
		    k=1;
			 j++;
		    i--;
       }
    }
    else 
       Buckets[((NoofElements_Bloc + 1) * j) + k++] = InputData[i];
  }
  Buckets[(NoofElements_Bloc + 1) * j] = k - 1;
      
  /**** Sending buckets to respective processors ****/

  BucketBuffer = (int *) malloc (sizeof (int) * (NoofElements + Numprocs));

  MPI_Alltoall (Buckets, NoofElements_Bloc + 1, MPI_INT, BucketBuffer, 
					 NoofElements_Bloc + 1, MPI_INT, MPI_COMM_WORLD);

  /**** Rearranging BucketBuffer ****/
  LocalBucket = (int *) malloc (sizeof (int) * 2 * NoofElements / Numprocs);

  count = 1;

  for (j=0; j<Numprocs; j++) {
  k = 1;
    for (i=0; i<BucketBuffer[(NoofElements/Numprocs + 1) * j]; i++) 
      LocalBucket[count++] = BucketBuffer[(NoofElements/Numprocs + 1) * j + k++];
  }
  LocalBucket[0] = count-1;
    
  /**** Sorting Local Buckets using Bubble Sort ****/
  /*qsort ((char *) InputData, NoofElements_Bloc, sizeof(int), intcompare); */

  NoElementsToSort = LocalBucket[0];
  qsort ((char *) &LocalBucket[1], NoElementsToSort, sizeof(int), intcompare); 

  /**** Gathering sorted sub blocks at root ****/
  if(MyRank == Root) {
  		OutputBuffer = (int *) malloc (sizeof(int) * 2 * NoofElements);
  		Output = (int *) malloc (sizeof (int) * NoofElements);
  }

  MPI_Gather (LocalBucket, 2*NoofElements_Bloc, MPI_INT, OutputBuffer, 
				  2*NoofElements_Bloc, MPI_INT, Root, MPI_COMM_WORLD);

  /**** Rearranging output buffer ****/
	if (MyRank == Root){
    stopT = MPI_Wtime();
    			printf("N: %d\n", NoofElements);
			printf("P: %d\n", Numprocs);

    printf("Time(sec): %f\n", stopT - startT);
    
		count = 0;
		for(j=0; j<Numprocs; j++){
          k = 1;
      	 for(i=0; i<OutputBuffer[(2 * NoofElements/Numprocs) * j]; i++) 
				 Output[count++] = OutputBuffer[(2*NoofElements/Numprocs) * j + k++];
    	}

      /**** Printng the output ****/
    	if ((fp = fopen("SampleSort.out", "a")) == NULL){
         	printf("Can't Open Output File \n");
      		exit(0);
    	}
		 if (MyRank == Root){
     fprintf(fp, "%d %d %f \n", NoofElements, Numprocs, stopT - startT);
     }

  // 	printf( "Sorted output sequence is\n\n");
  //   	for (i=0; i<NoofElements; i++){
	      
	//       	printf( "%d   ", Output[i]);
	// }
	printf ( " \n " );
    	free(Input);
  	free(OutputBuffer);
  	free(Output);
   }/* MyRank==0*/

  	free(InputData);
  	free(Splitter);
  	free(AllSplitter);
  	free(Buckets);
  	free(BucketBuffer);
  	free(LocalBucket);

   /**** Finalize ****/
   MPI_Finalize();
}



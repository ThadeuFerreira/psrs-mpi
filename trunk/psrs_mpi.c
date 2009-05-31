/* Para compilar: mpicc psrs_mpi.c -o psrs_mpi 
   Para executar:  mpiexec -n 10 ./psrs_mpi 100
   */
#include <stdio.h>
#include <mpi.h>
#include <time.h>

	int *vet; /*Data Vector*/

int master(int size, int sizeVector){
	printf("I am your master, I own %d salves, sizeVector = %d\n", size - 1, sizeVector);
	int i;
	for( i = 0 ; i < sizeVector ; i++ ){
        	printf("%d ", vet[i]);
     	}
	return 0;
}

int slave(int rank){
	printf("I am a slave, my number is %d\n", rank);
}

int main (argc, argv)
     int argc;
     char *argv[];
{

	int rank, numThreads;
	int sizeVector = atoi(argv[1]); /*Size of data vector*/
	
	vet = malloc(sizeVector*sizeof(int)); /*Dinamic allocation*/
	srand (time(NULL));
    	
	int i;
	for( i = 0 ; i < sizeVector ; i++ ){
        	vet[i] = (rand()*(1e2/RAND_MAX))+1;
     	}

	MPI_Init (&argc, &argv);	/* starts MPI */
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);	/* get current process id */
	MPI_Comm_size (MPI_COMM_WORLD, &numThreads);	/* get number of processes */
	
	if (rank==0){
		master(numThreads, sizeVector);
	}
	else{
		slave(rank);
	}
	
	MPI_Finalize();
 
	return 0;
}


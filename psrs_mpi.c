/* Para compilar: mpicc psrs_mpi.c -o psrs_mpi 
   Para executar:  mpiexec -n 10 ./psrs_mpi 100
   */
#include <stdio.h>
#include <mpi.h>

int master(int size){
	printf("I am your master, I own %d salves\n", size);
	return 0;
}

int slave(int rank){
	printf("I am a slave, my number is %d\n", rank);
}
int main (argc, argv)
     int argc;
     char *argv[];
{
	int *vet; /*Data Vector*/
	int rank, numThreads;
	int sizeVector = atoi(argv[1]); /*Size of data vector*/
	
	vet = malloc(sizeVector*sizeof(int)); /*Dinamic allocation*/

	MPI_Init (&argc, &argv);	/* starts MPI */
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);	/* get current process id */
	MPI_Comm_size (MPI_COMM_WORLD, &numThreads);	/* get number of processes */
	
	if (rank==0){
		master(numThreads);
	}
	else{
		slave(rank);
	}
	
	MPI_Finalize();
 
	return 0;
}


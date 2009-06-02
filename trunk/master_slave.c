/* Para compilar: mpicc master_slave.c -o master_slave 
   Para executar:  mpiexec -n 10 ./master_slave 100
   */
#include <stdio.h>
#include <mpi.h>
#include <time.h>

	MPI_Status Stat;
	int *vet; /*Data Vector*/
	int tag = 1;

int master(int size, int sizeVector){

	int i, out, dest;
	int sizeTempVector1 = sizeVector/numThreads;
	int restTempVector1 = sizeVector%numThreads;

	for( i = 0 ; i < sizeVector ; i++ ){
        	printf("%d ", vet[i]);
     	}
	printf("\n");
	for(i = 1; i < size; i++){
		out = 0;
		dest = i;

		MPI_Send(&out, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
	}	
	
	slave(0, sizeVector);

	MPI_Bcast(&out, 1, MPI_INT, 0, MPI_COMM_WORLD);
	return 0;
}

int slave(int rank, int sizeVector){
	int source = 0;
	int in;
	int *newVet; /*Data Vector*/
	MPI_Recv(&in, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &Stat);

	MPI_Bcast(&in, 1, MPI_INT, 0, MPI_COMM_WORLD);

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
		slave(rank, sizeVector);
	}
	
	MPI_Finalize();
 
	return 0;
}


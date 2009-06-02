/* Para compilar: mpicc master_slave.c -o master_slave 
   Para executar:  mpiexec -n 10 ./master_slave 100
   */
#include <stdio.h>
#include <mpi.h>
#include <time.h>

	MPI_Status Stat;
	int *vet; /*Data Vector*/
	int tag = 1;

int master(int numThreads, int sizeVector){

	int i, dest;
	int sizeTempVector1 = sizeVector/numThreads; /*First division inter process*/
	int restTempVector1 = sizeVector%numThreads; /*Rest of last process*/

	for( i = 0 ; i < sizeVector ; i++ ){
        	printf("%d ", vet[i]);
     	}
	printf("\n");
	for(i = 1; i < numThreads; i++){

		dest = i;
		if(i == (numThreads-1)){
			MPI_Send(&restTempVector1, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);		
		}
		MPI_Send(&sizeTempVector1, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
	}
	
	quik_sort(0, sizeTempVector1, 0);
	//slave(0);

	return 0;
}

int slave(int rank, int numThreads){
	int source = 0;
	int sizeTempVector1, restTempVector1 = 0;
	if(rank ==(numThreads - 1))	
		MPI_Recv(&restTempVector1, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &Stat);

	MPI_Recv(&sizeTempVector1, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &Stat);
	printf("SLAVE = %d ** Tamanho = %d ** Resto = %d\n", rank, sizeTempVector1, restTempVector1);
	quik_sort(rank,sizeTempVector1, restTempVector1);

return 0;
}

int quik_sort(int rank, int sizeVector, int rest){
	int *newVet; /*Data Vector*/	
	int inicio, fim;
	newVet = malloc(sizeVector*sizeof(int)); /*Dinamic allocation*/
	int begin = rank*sizeVector;
	int end = begin +  sizeVector + rest;
	printf("SLAVE = %d ** INICIO = %d ** FIM = %d\n", rank, begin, end);
	int i, j = 0;

	for(i = begin; i < end; i++){		
		newVet[j] = vet[i];
		j++; 
	}

	for(i = 0; i < (sizeVector + rest); i ++){
		printf("Rank = %d\t Indice = %d\t Elemento = %d\n", rank, i, newVet[i]);
	}
	return 0;
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
		slave(rank, numThreads);
	}
	
	MPI_Finalize();
 
	return 0;
}


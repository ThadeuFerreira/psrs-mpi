/* Para compilar: mpicc master_slave.c -o master_slave 
   Para executar:  mpiexec -n 10 ./master_slave 100
   */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

	MPI_Status Stat;

	int *vet; /*Data Vector*/
	int tag = 1;

int comp(const int * a,const int * b)
{
  if (*a==*b)
    return 0;
  else
    if (*a < *b)
        return -1;
     else
      return 1;
}

int master(int numThreads, int sizeVector){

	int i, j, pass;
	
	int *regSamp, *tempSamp; /*Regular Sample array*/
	int *pivots;
	regSamp = malloc(numThreads*numThreads*sizeof(int));
	tempSamp = malloc(numThreads*sizeof(int));
	pivots = malloc((numThreads-1)*sizeof(int));
	int sizeTempVector = sizeVector/numThreads; /*First division inter process*/
	int restTempVector = sizeVector%numThreads; /*Rest of last process*/

	for( i = 0 ; i < sizeVector ; i++ ){
        	printf("%d-", vet[i]);
     	}
	printf("\n");
	for(i = 1; i < numThreads; i++){
		if(i == (numThreads-1)){
			MPI_Send(&restTempVector, 1, MPI_INT, i, tag, MPI_COMM_WORLD);		
		}
		MPI_Send(&sizeTempVector, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
	}
	
	int *newVet; /*Data Vector*/	
	newVet = malloc((sizeTempVector + restTempVector)*sizeof(int)); /*Dinamic allocation*/
	
	phase1(0, sizeTempVector, 0, numThreads, newVet);

	for(i = 0; i < numThreads; i++){

		MPI_Recv(tempSamp, numThreads , MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &Stat);
	
		for(j = 0; j < numThreads; j++){		
			printf("tempSamp = %d \n", tempSamp[j]); 
			regSamp[i*numThreads + j] = tempSamp[j];		
		}
	}

	free(tempSamp);

	qsort(regSamp, numThreads*numThreads, sizeof(int), comp); /*Sequential QuickSort*/

	pass = numThreads*numThreads/(numThreads);

	for(i = 0; i < (numThreads-1); i++){
		pivots[i] = regSamp[pass*(i+1)]; /*Regular Sampling*/
	}
	free(regSamp);

	for(i = 0; i < (numThreads-1); i++){
		printf("Pivots = %d\n",pivots[i]);
	}

	MPI_Bcast(pivots, (numThreads-1), MPI_INT, 0, MPI_COMM_WORLD);

	phase2(0, numThreads, newVet, pivots);
	return 0;
}

int slave(int rank, int numThreads){
	int source = 0;
	int sizeTempVector, restTempVector = 0;
	if(rank ==(numThreads - 1))	
		MPI_Recv(&restTempVector, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &Stat);

	MPI_Recv(&sizeTempVector, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &Stat);
	int *newVet; /*Data Vector*/	
	newVet = malloc((sizeTempVector + restTempVector)*sizeof(int)); /*Dinamic allocation*/
	phase1(rank, sizeTempVector, restTempVector, numThreads, newVet);
	
	int *pivots;
	pivots = malloc((numThreads-1)*sizeof(int));
	MPI_Bcast(pivots, (numThreads-1), MPI_INT, 0, MPI_COMM_WORLD);
	phase2(rank, numThreads, newVet, pivots);

return 0;
}

int phase1(int rank, int sizeVector, int rest, int numThreads, int *newVet){

	int *regSamp; /*Regular Sample array*/
	int inicio, fim;
	
	regSamp = malloc(numThreads*sizeof(int));
	int begin = rank*sizeVector;
	int end = begin +  sizeVector + rest;

	int i, j = 0;

	for(i = begin; i < end; i++){		
		newVet[j] = vet[i];
		j++; 
	}

	qsort(newVet, sizeVector + rest, sizeof(int), comp); /*Sequential QuickSort*/
	int pass = (sizeVector + rest)/numThreads;

	for(i = 0; i < numThreads; i++){
		regSamp[i] = newVet[pass*(i)]; /*Regular Sampling*/
	}
	for(i = 0; i < (sizeVector + rest); i ++){
		printf("Rank = %d\t Indice = %d\t Elemento = %d\n", rank, i, newVet[i]);
	}
	for(i = 0; i < numThreads; i++){
		printf("****SAMPLING **** Rank = %d\t PASS = %d\t Elemento = %d\n", rank, pass, regSamp[i]);
	}

	MPI_Send(regSamp, numThreads, MPI_INT, 0, tag, MPI_COMM_WORLD);
	free(regSamp);
		
	return 0;
}

int phase2(int rank, int numThreads, int *newVet, int *pivots){
	int cont, i;
	int *sendBuff;
	int dest = 0;
	int size = sizeof(newVet);

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


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

	int i, j, dest;
	int *regSamp, *tempSamp; /*Regular Sample array*/
	regSamp = malloc(numThreads*numThreads*sizeof(int));
	tempSamp = malloc(numThreads*sizeof(int));
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
	
	phase1(0, sizeTempVector1, 0, numThreads);
	for(i = 0; i < numThreads; i++){

		MPI_Recv(&tempSamp, numThreads , MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &Stat);
	printf("\nMASTER RECEBEU %d .. %d\n", i, numThreads);
		for(j = 0; j < numThreads; j++){
			printf("j = %d\n", j);/*Correto*/
			printf("tempSamp = %d \n", tempSamp[j]); /*dando erro*/
			//regSamp[i*numThreads + j] = tempSamp[j];		
		}
		

	}
/*
	for(i = 0; i < numThreads*numThreads; i ++){
	printf("REGULAR SAMPLING FINAL = %d", regSamp[i]);
	}
*/


	return 0;
}

int slave(int rank, int numThreads){
	int source = 0;
	int sizeTempVector1, restTempVector1 = 0;
	if(rank ==(numThreads - 1))	
		MPI_Recv(&restTempVector1, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &Stat);

	MPI_Recv(&sizeTempVector1, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &Stat);

	phase1(rank, sizeTempVector1, restTempVector1, numThreads);

return 0;
}

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

int phase1(int rank, int sizeVector, int rest, int numThreads){
	int *newVet; /*Data Vector*/	
	int *regSamp; /*Regular Sample array*/
	int inicio, fim;
	newVet = malloc(sizeVector*sizeof(int)); /*Dinamic allocation*/
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


		MPI_Send(&regSamp, numThreads, MPI_INT, 0, tag, MPI_COMM_WORLD);
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


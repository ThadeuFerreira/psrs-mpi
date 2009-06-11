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
		
			MPI_Send(&restTempVector, 1, MPI_INT, i, 0, MPI_COMM_WORLD);		
		
		MPI_Send(&sizeTempVector, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
	}
	
	int *newVet; /*Data Vector*/	
	
	newVet = malloc(sizeTempVector*sizeof(int)); /*Dinamic allocation*/

	phase1(0, sizeTempVector, 0, numThreads, newVet);

	for(i = 0; i < numThreads; i++){

		MPI_Recv(tempSamp, numThreads , MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &Stat);
	
		for(j = 0; j < numThreads; j++){		
			
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

	MPI_Bcast(pivots, (numThreads-1), MPI_INT, 0, MPI_COMM_WORLD);

	phase2(0, numThreads, newVet, sizeTempVector, restTempVector, pivots);
	for( i = 0 ; i < sizeVector ; i++ ){
        	printf("%d-", vet[i]);
     	}
	return 0;
}

int slave(int rank, int numThreads){
	int source = 0;
	int sizeTempVector, restTempVector = 0;
	int newSize;
	
	MPI_Recv(&restTempVector, 1, MPI_INT, source, 0, MPI_COMM_WORLD, &Stat);

	MPI_Recv(&sizeTempVector, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &Stat);
	int *newVet; /*Data Vector*/	
	if(rank ==(numThreads - 1))
	{
		newSize = sizeTempVector + restTempVector;
	}else
	{
		newSize = sizeTempVector;
	}
	
	newVet = malloc(newSize*sizeof(int)); /*Dinamic allocation*/
	phase1(rank, sizeTempVector, restTempVector, numThreads, newVet);
	
	int *pivots;
	pivots = malloc((numThreads-1)*sizeof(int));
	MPI_Bcast(pivots, (numThreads-1), MPI_INT, 0, MPI_COMM_WORLD);
	phase2(rank, numThreads, newVet, sizeTempVector, restTempVector, pivots);
	

return 0;
}

int phase1(int rank, int sizeVector, int rest, int numThreads, int *newVet){

	int *regSamp; /*Regular Sample array*/
	int inicio, fim;
	
	regSamp = malloc(numThreads*sizeof(int));
	int begin = rank*sizeVector;
	if(rank != numThreads - 1) 
		rest = 0;
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

	MPI_Send(regSamp, numThreads, MPI_INT, 0, 2, MPI_COMM_WORLD);
	free(regSamp);
		
	return 0;
}

int phase2(int rank, int numThreads, int *newVet, int newSize, int restTempVector, int *pivots){
	int dest = 0;
	int i = 0;
	int j = 0;
	int m;
	int cont;
	int *sendBuff, *recvBuff, *finalBuff;
	int maxSize = newSize*numThreads + restTempVector;
	sendBuff = malloc((newSize + restTempVector)*sizeof(int));
	recvBuff = malloc((newSize + restTempVector)*sizeof(int));
	finalBuff = malloc(maxSize*sizeof(int));
	printf("RANK = %d NUMTHREADS = % d Tamanhho1 = %d Tamanhho2 = %d Rest = %d\n", rank, numThreads, newSize,maxSize, restTempVector);
	for(i = 0; i < newSize; i++) {
	sendBuff[i] = -1;
	recvBuff[i] = -1;
	finalBuff[i] = 0;
	}

	int k;
	i = 0;
	m = 0;
	for(k = 0; k < numThreads; k++){
		while(((newVet[i] <= pivots[dest])||(dest == (numThreads -1)))&&(i < newSize)){
			sendBuff[j] = newVet[i];
			i++;
			j++;
		}
		MPI_Send(sendBuff, j, MPI_INT, dest, 3, MPI_COMM_WORLD);
		MPI_Recv(recvBuff, newSize , MPI_INT, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, &Stat);

		j = 0;
		cont = 0;
		while(recvBuff[cont] != -1){
			finalBuff[m] = recvBuff[cont];			
			m++;
			cont++;
		}
		for(cont = 0; cont <= newSize; cont++){
			recvBuff[cont] = -1;
			sendBuff[cont] = -1;
		}		
		dest++;		
	}
		qsort(finalBuff, m, sizeof(int), comp); /*Sequential QuickSort*/

		for(i = 0 ; i < m; i ++) printf("RANK = %d  ---  FinalBuff[%d] = %d\n", rank,  i, finalBuff[i]);
	
		MPI_Gather(finalBuff,m,MPI_INT,vet, m,MPI_INT,0,MPI_COMM_WORLD);
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


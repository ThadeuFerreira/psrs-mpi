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
	
	phase1(0, sizeTempVector1, 0);
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
	phase1(rank,sizeTempVector1, restTempVector1);

return 0;
}

int phase1(int rank, int sizeVector, int rest){
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

	qsort_seq(newVet, begin, end);

	for(i = 0; i < (sizeVector + rest); i ++){
		printf("Rank = %d\t Indice = %d\t Elemento = %d\n", rank, i, newVet[i]);
	}

	return 0;
}
void qsort_seq(int array[], int begin, int end) {
   if(end - begin > 0) {
    int aux;
    int pivot = array[begin];
    int left = begin + 1;
    int right = end;
    while(left < right) {
        if(array[left] <= pivot) {
            left++;
        } else {
           // Troca o valor de array[left] com array[right]
           aux = array[left];
           array[left] = array[right];
           array[right] = aux;
           // Fim da troca ( equivale a fun��o swap(array[left], array[right]) )
           right--;
        }
    }
    if(array[left] > pivot) {
        left--;
    }
                                         
    // Troca o valor de array[begin] com array[left]
    aux = array[begin];
    array[begin] = array[left];
    array[left] = aux;
    // Fim da troca ( equivale a fun��o swap(array[begin], array[left]) )
    // Faz as chamadas recursivas para as duas partes da lista
    qsort_seq(array, begin, left-1);
    qsort_seq(array, right, end);
   }
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


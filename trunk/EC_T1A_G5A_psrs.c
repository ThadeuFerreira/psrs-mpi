/* Para compilar: mpicc EC_T1A_G5A_psrs.c -o EC_T1A_G5A_psrs 
   Para executar:  mpiexec -n [numero de Threads] ./EC_T1A_G5A_psrs [Tamanho do Vetor]
   */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

	MPI_Status Stat;

	int *vet; /*Vetor principal de Dados*/
	int tag = 1;

/*Funcao de comparacao para o quicksort sequancial*/
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

/*Funcao mestre, executada pela Thread ZERO*/
int master(int numThreads, int sizeVector){

	int i, j, pass;
	
	int *regSamp, *tempSamp; 
	int *pivots;

	/*Regular Sample array*/
	regSamp = malloc(numThreads*numThreads*sizeof(int));

	/*Vetor temporario para os Samplins que cada Thread envia para o mestre*/
	tempSamp = malloc(numThreads*sizeof(int)); 

	/*Vetor de pivots definitivo*/
	pivots = malloc((numThreads-1)*sizeof(int));
	int sizeTempVector = sizeVector/numThreads; /*First division inter process*/
	int restTempVector = sizeVector%numThreads; /*Rest of last process*/

	/*Envia para cada uma das Threads o tamanho que os vetores temporarios terão*/
	for(i = 1; i < numThreads; i++){
		MPI_Send(&restTempVector, 1, MPI_INT, i, 0, MPI_COMM_WORLD);		
		MPI_Send(&sizeTempVector, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
	}
	
	int *newVet; /*Vetor de intermediario*/		
	newVet = malloc(sizeTempVector*sizeof(int));

	/*Chamada para a primeira faze de ordenacao*/
	phase1(0, sizeTempVector, 0, numThreads, newVet);

	/*Recebimento das amostras das threads*/
	for(i = 0; i < numThreads; i++){
		/*Aqui a ordem de recebimento não faz diferenca*/
		MPI_Recv(tempSamp, numThreads , MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &Stat);
		for(j = 0; j < numThreads; j++){		
			regSamp[i*numThreads + j] = tempSamp[j];		
		}
	}

	free(tempSamp);

	/*Ordenacao das amostras totais*/
	qsort(regSamp, numThreads*numThreads, sizeof(int), comp); /*Sequential QuickSort*/
	
	/*tamanho do passo a ser dado para a escolha de bons Pivots*/
	pass = numThreads*numThreads/(numThreads); 

	for(i = 0; i < (numThreads-1); i++){
		pivots[i] = regSamp[pass*(i+1)]; /*Regular Sampling*/
	}
	free(regSamp);

	MPI_Bcast(pivots, (numThreads-1), MPI_INT, 0, MPI_COMM_WORLD);/*Transmicao dos pivots*/
	


	/*Inicio da segunda fase*/	
	phase2(0, numThreads, newVet, sizeTempVector, restTempVector, pivots);
	free(pivots);
	/*Buffer para o recebimento dos vetores ordenados a partir do pivots*/
	int *finalBuff = malloc(sizeVector*sizeof(int));

	int cont_1, cont_2;
	cont_2 = 0;
	for (j = 0; j < numThreads; j++){	

		for( i = 0 ; i < sizeVector ; i++ ) 	finalBuff[i] = -1;
     		/*Recebe de cada uma das threads em ordem*/
		MPI_Recv(finalBuff, sizeVector , MPI_INT, j, 4, MPI_COMM_WORLD, &Stat);
		//for( i = 0 ; i < sizeVector ; i++ ) if(finalBuff[i] != -1) printf("Rank = %d final buff [%d] = %d\n", j, i, finalBuff[i]);
	 		cont_1 = 0;
			while((finalBuff[cont_1]!= -1)&&(cont_2 <sizeVector)){
				vet[cont_2] = finalBuff[cont_1];	/*Insere no vetor definitivo*/
				cont_1++;
				cont_2++;
			}	
     	
	}
	free(finalBuff);


	return 0;
}

/*Funcao Chamada pelas Threads escravas*/
int slave(int rank, int numThreads){
	int source = 0;
	int sizeTempVector, restTempVector; /*Tamanhos dos vetores temporarios*/
	int newSize;
	
	/*Espera o recebimento dos dados da thread Mestre*/
	MPI_Recv(&restTempVector, 1, MPI_INT, source, 0, MPI_COMM_WORLD, &Stat);
	MPI_Recv(&sizeTempVector, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &Stat);

	int *newVet; /*Vetor intermediario*/	
	if(rank ==(numThreads - 1))
	{
		/*o resto do vetor é tratado pela ultima thread*/
		newSize = sizeTempVector + restTempVector; 
	}else
	{
		newSize = sizeTempVector;
	}
	
	newVet = malloc(newSize*sizeof(int)); 
	phase1(rank, sizeTempVector, restTempVector, numThreads, newVet);

	int *pivots;
	pivots = malloc((numThreads-1)*sizeof(int));
	/*Recebe o vetor de pivots do Mestre*/
	MPI_Bcast(pivots, (numThreads-1), MPI_INT, 0, MPI_COMM_WORLD);

	/*Inicio da segunda e ultima fase para os escravos*/
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

	/*O vetor intermediario recebe uma parte do vetor principal*/
	for(i = begin; i < end; i++){		
		newVet[j] = vet[i];
		j++; 
	}

	/*Vetor intermediario é ordenado sequencialmente*/
	qsort(newVet, sizeVector + rest, sizeof(int), comp); /*Sequential QuickSort*/
	
	int pass = (sizeVector + rest)/numThreads;/*Passo a ser dado para uma boa amostragem*/

	for(i = 0; i < numThreads; i++){
		regSamp[i] = newVet[pass*(i)]; /*Amostragem regular*/
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
	/*Vetor de envio para a ultima ordenacao*/
	sendBuff = malloc((newSize + restTempVector)*sizeof(int));
	/*Vetor de recebimento*/
	recvBuff = malloc((newSize + restTempVector)*sizeof(int));
	/*Vetor final*/
	finalBuff = malloc(maxSize*sizeof(int));
	
	/*Caso se trate da ultima thread e haja resto na divisão de tarefas*/
	if(rank == (numThreads - 1))
		 newSize = newSize + restTempVector;
	
	/*Inicializacao dos vetores*/
	/*Importante pois não é sabido o tamanho que cada um vai receber*/
	for(i = 0; i < newSize; i++) {
	sendBuff[i] = -1;
	recvBuff[i] = -1;
	finalBuff[i] = 0;
	}
	for( i = 0 ; i < (numThreads -1) ; i++ ) printf("Rank = %d final buff [%d] = %d\n", rank, i, pivots[i]);

	int k;
	i = 0;
	m = 0;
	for(k = 0; k < numThreads; k++){

		/*separacao do vetor em relacao ao pivot a ser enviado*/
		while(((newVet[i] <= pivots[dest])||(dest == (numThreads -1)))&&(i < newSize)){
			sendBuff[j] = newVet[i];
			i++;
			j++;
		}
		/*envio do Buffer para uma thread*/
		/*o envio é feito em ordem, mas ele não precisaria ser bloqueante*/
		MPI_Send(sendBuff, j, MPI_INT, dest, 3, MPI_COMM_WORLD);

		/*Recebimento de um parte do vetor final*/
		/*não é preciso seguir uma orde de recebimento*/
		MPI_Recv(recvBuff, newSize , MPI_INT, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, &Stat);

		j = 0;
		cont = 0;
		/*incercao no vetor intermediario final*/
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
		/*Ordenacão do vetor com todos os dados recebido*/
		qsort(finalBuff, m, sizeof(int), comp); /*Sequential QuickSort*/
		
		/*Envio para o Mestre*/
		MPI_Send(finalBuff, m, MPI_INT, 0, 4, MPI_COMM_WORLD);
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
        	vet[i] = (rand()*(1e2/RAND_MAX))+1; /*Inicializacao do Vetor principal*/
     	}

	MPI_Init (&argc, &argv);	/* starts MPI */
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);	/* get current process id */
	MPI_Comm_size (MPI_COMM_WORLD, &numThreads);	/* get number of processes */
	
	/*Se o numero de threads sor muito proximo ao tamanho do vetor, nao vale a pena fazer o PSRS*/
	if(sizeVector/numThreads <= 1){
		if(rank==0){
			/*imprime o vetor principal, ainda desordenado*/
			printf("\n\t\t-----------------\n\t\tVetor desordenado\n\t\t----------------\n");
			for( i = 0 ; i < sizeVector ; i++ ){		
        			printf("%d-", vet[i]); 
     			}
			printf("\n");

		 	qsort(vet, sizeVector, sizeof(int), comp);

			/*Imprime o vetor já ordenado*/
			printf("\n\t\t***************\n\t\tVetor ordenadoO\n\t\t***************\n");
			for( i = 0 ; i < sizeVector ; i++ ) printf("%d*", vet[i]);
			printf("\n");
		}
	}else{
		if (rank==0){
			/*imprime o vetor principal, ainda desordenado*/
			printf("\n\t\t-----------------\n\t\tVetor DESORDENADO\n\t\t----------------\n");
			for( i = 0 ; i < sizeVector ; i++ ){		
        			printf("%d-", vet[i]); 
     			}
			printf("\n");

			master(numThreads, sizeVector);

			/*Imprime o vetor já ordenado*/
			printf("\n\t\t***************\n\t\tVetor ORDENADO\n\t\t***************\n");
			for( i = 0 ; i < sizeVector ; i++ ) printf("%d*", vet[i]);
			printf("\n");
		}
		else{
			slave(rank, numThreads);
		}
	}
	MPI_Finalize();
 
	return 0;
}


#include <mpi.h>
#include <stdio.h>

int main(argc,argv) 
int argc;
char *argv[];  {
int numtasks, rank, dest, source, rc, count, tag=1;  
char inmsg, outmsg='x';
MPI_Status Stat;

MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);


source = rank-1;
dest = rank+1;
if (rank == 0)  {
source = numtasks - 1;
printf("PRIMEIRO +++++++ \n");
outmsg = 'P';
}
if (rank == (numtasks - 1)){
  dest = 0;
	printf("ULTIMO +++++++ \n");
outmsg = 'U';
}  
rc = MPI_Send(&outmsg, 1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
  rc = MPI_Recv(&inmsg, 1, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);

	printf("Recebe = %c ", inmsg);
  

rc = MPI_Get_count(&Stat, MPI_CHAR, &count);
printf("Task %d: Received %d char(s) from task %d with tag %d \n",
       rank, count, Stat.MPI_SOURCE, Stat.MPI_TAG);


MPI_Finalize();
return 0;
}


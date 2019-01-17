/*
Adam Castillo
5/3/17
commands:
mpic++ CS176_prog5.cpp 
 mpiexec -n 2 ./a.out 400 >x2
  mpiexec -n 4 ./a.out 400 >x4
  mpiexec -n 8 ./a.out 400 >x8
*/
#include <cstdlib>
#include <iostream>
#include <mpi.h>
using namespace std; 

int Compare(const void* a_p, const void* b_p); 
void Merge(int local_A[],int local_B[], int local_n);

int main(int argc,  char** argv)
{
	int my_rank,  comm_sz;
	int n, local_n;
	int* local_A; 
	
	MPI_Init(NULL,NULL); // start up with MPI
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank); // get my process rank
	MPI_Comm_size(MPI_COMM_WORLD,&comm_sz); //  get number of processes
	
	////////////////////////////////////////////////////////////////
	if(my_rank == 0)
	{
		n = atoi(argv[1]); 
		if(n%comm_sz != 0)
		{
			cerr << "n should be evenly divisible by "<< comm_sz << endl;  
			MPI_Finalize();
			exit(1);
		}
		for(int dest =1; dest <comm_sz; dest++)
			MPI_Send(&n,1,MPI_INT,dest,0,MPI_COMM_WORLD); 
	}
	else 
	{
		MPI_Recv(&n,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE); 
	}
	//////////////////////////////////////////////////////////////////
	
	local_n = n/comm_sz; 
	local_A = new int[n]; 
	
	srandom(my_rank+1); //without this, random #'s generated in processes are identical 
	for(int i =0; i < local_n; i++)
		local_A[i]= random() %100;  // all values under 100
	
	//display unsorted local list in each process 
	
	cout <<"Process_" << my_rank <<",local list:" << endl;  
	for(int i = 0;  i < local_n; i++)
		cout << " "<< local_A[i]; 
	cout << endl;  
	
	//local list sorting in each process 
	qsort(local_A,local_n, sizeof(int), Compare);
	
	//reduction(merge) part 
	int partner; // for tree reduction 
	int done =0; //eliminate while loop in each process 
	int *local_B; //recieve list for merging
	MPI_Status status; 
	
	int divisor = 2; 
	int core_difference = 1;
	while(done == 0 && divisor <= comm_sz)
	{
		if(my_rank% divisor == 0)
		{
			local_B = new int[n]; // used to receive local_a
			
			partner =  my_rank + core_difference; 
			MPI_Recv(local_B,n,MPI_INT,partner,0,MPI_COMM_WORLD,&status); //recieve from partner 
			
			Merge(local_A,local_B,local_n);
			local_n = 2*local_n; //size now doubled
			
			delete[] local_B; 
		}
		else //sender process 
		{
			partner = my_rank - core_difference; 
			MPI_Send(local_A,local_n, MPI_INT, partner,0,MPI_COMM_WORLD);
			done = 1;
		}
		divisor *=2; 
		core_difference *=2; 
	}
	////////////// reduction ends 
	
	////in process one , display global list 
	if(my_rank == 0)
	{
		cout << "Sorted Global List: \n"; 
		for(int i = 0;  i < n; i++)
			cout << local_A[i] <<  " "; 
		cout << endl;
	}
	
	delete[] local_A;
	MPI_Finalize(); 
	return 0;  
}


/////Merge function needed for qsort
int Compare(const void* a_p, const void* b_p)
{
	int a = *(int*)a_p; 
	int b = *(int*)b_p; 
	if(a<b)return -1; 
	else if (a==b)return 0; 
	else return 1;
}

void Merge(int local_A[], int local_B[],  int local_n)
{
	int ai,bi,ci;
	int csize = 2*local_n;
	int* temp_C = new int[csize]; 
	
	ai = 0; bi = 0; ci = 0; 
	while(ai < local_n && bi < local_n)
	{
		if(local_A[ai]<= local_B[bi])
		{
			temp_C[ci] = local_A[ai];
			ci++; ai++;  
		}
		else
		{
			temp_C[ci] =local_B[bi]; 
			ci++; bi++;
		}
	}
	if(ai >= local_n)
	{
		for(int i = ci; i < csize; i++,bi++)
			temp_C[i] = local_B[bi];
	}
	else if (bi >= local_n)
	{
		for(int i = ci; i < csize; i++, ai++)
			temp_C[i] = local_A[ai]; 
	}
	for(int i = 0; i< csize; i++)
		local_A[i] = temp_C[i];  
	delete[] temp_C;
}

#include <iostream>
#include <random>
#include <cstdlib>
#include<ctime>
#include <omp.h>
using namespace std; 

int compare(const void*,  const void*);
int merge(int arr[], int,  int, int);
bool sorted(int arr[]);
int numElements, numThreads;
char *flag; 
int *chunks;

int main(int argc, char ** argv)
{
	if(argv[1] && argv[2] && argv[3])
	{
	int *arr; 	
	double start,end; 
	numThreads = atoi(argv[1]);
	numElements = atoi(argv[2]);
	flag  = argv[3];
	arr =  new int[numElements]; 
	chunks = new int[numThreads];
	
	srand(time(0));
	#pragma omp parallel for num_threads(numThreads)
		for(int i = 0;  i < numElements; i++)
			arr[i] =  rand() % numElements + 1; // 1.. number elements  
	
	if(*flag == 'p')
	{
	for(int i = 0; i < numElements; i++)
		cout << arr[i] << " ";
	cout << endl; 
	}
	start = omp_get_wtime();
	#pragma omp parallel num_threads(numThreads)
	{
		int eleTotal, myFirst, myLast;
		int quotient = numElements / numThreads;
		int remainder = numElements % numThreads;
		int rank = omp_get_thread_num();
		eleTotal = numElements/numThreads;
		myFirst = rank * eleTotal;
		myLast = myFirst + eleTotal - 1;
		int localList[eleTotal]; // createlist with big enough size 
		chunks[rank] = eleTotal;
		//copy to localList
		#pragma omp parallel for num_threads(numThreads) 
			for(int i = 0;  i < eleTotal; i++)
				localList[i] = arr[i+myFirst]; 
		
		//sort localList
		qsort(localList, eleTotal, sizeof(int), compare);
		
		//copy  back 
		#pragma omp parallel for num_threads(numThreads) 
			for(int i = 0;  i < eleTotal; i++)
				arr[i+myFirst] = localList[i];
		
		if(*flag == 'p')
		{
		#pragma omp critical
		{		
		cout << "sorted " << rank << ": ";  
		for(int i = 0; i < eleTotal; i++)
			cout << localList[i] << " ";
		cout << endl;
		}
		}

		int divisor =  2, coreDifference = 1;
		while(divisor <= numThreads)
		{
			#pragma omp barrier
			if(rank % divisor == 0)
			{
				int partner =  rank + coreDifference;
				merge(arr, myFirst, rank, partner);
			}
			divisor *= 2; coreDifference *= 2;
		}
	}	
	end = omp_get_wtime();
	
	if(*flag == 'p')
	{
	for(int i = 0; i < numElements; i++)
		cout << arr[i] << " ";
	cout << endl; 
	}
	
	cout << "Time elapsed: " << (end - start) <<endl;
	(sorted(arr))? cout << "Verified  sorted \n" : cout << "Not sorted \n";
	return 0;	
	}
	else 
	{
		cout << "Invalid Command Line Arguments" << endl;
	}
}

int merge(int arr[],int start, int rank, int partRank)
{
	int partChunk, partFirst, partLast;
	int quotient = numElements / numThreads;
	int remainder = numElements % numThreads;
	partChunk = numElements/numThreads;
	partFirst = partRank * partChunk;
	
	int size = chunks[rank] + chunks[partRank];
	int temp[size] = {}; // list should be able to fit partner 
 	//............... Begin merge ................//
	int i = start;
	int j = partFirst;
	int k = 0;
		
	while(k < size)
	{
		if(arr[i] <= arr[j] && i < start + chunks[rank])
			temp[k++] = arr[i++];
		else if( arr[j] < arr[i] && j < partFirst +chunks[partRank])
			temp[k++] = arr[j++];
		else if(i >= start + chunks[rank])
			 temp[k++] = arr[j++];
		else 
			 temp[k++] = arr[i++];
	}

		for(int i = 0;  i < size; i++)
			arr[i+start] = temp[i]; 

	chunks[rank] += chunks[partRank]; //  update length of sorted partition
	}

	
	
	
bool sorted(int arr[])
{
	for(int i=0; i < numElements-1; i++)
	{
		if(arr[i] > arr[i+1])
			return false;  
	}
	return true; 
}
	
	
// This function is needed for quicksort
int compare(const void *aP ,  const void*bP)
{
	int a =  *((int*)aP);
	int b =  *((int*)bP);
	if (a < b) return -1;
	else if (a == b) return 0;
	else return 1;  
}
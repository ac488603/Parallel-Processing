/*
Adam Castillo
4/18/17
This program implements mergesort using shared memory parallelism. The
library used is openmp. The program takes three command line arguments.
The first is the the number of threads. The second is the number of elements
to be sorted and the third is the print flag. If 'p' is passed as the print flag argument 
then the begining array, individual sorted arrays, and final sorted arrays will 
be displayed on the screen. Any other character will not display the arrays. The program 
sorts any problem size(unevenly and evenly divisible). 
os: windows
commands: shown in typescript
*/
#include <iostream>
#include <random>
#include <cstdlib>
#include <omp.h>
#include <ctime>
using namespace std; 

// function prototypes
int compare(const void*,  const void*);
int merge(int arr[], int,  int, int);
bool sorted(int arr[]);
// globals 
int numElements, numThreads;
int *chunks; // number of elements that starting pos has (used in merge) 
char *flag; // print flag 

int main(int argc, char ** argv)
{
	if(argv[1] && argv[2] && argv[3]) // check to see if all arguments are passed 
	{
	int *arr;  //  array to be sorted 
	double start,end;  //  for time checking 
	// initializing values 
	numThreads = atoi(argv[1]);  
	numElements = atoi(argv[2]);
	flag  = argv[3];	
	arr =  new int[numElements]; 
	chunks = new int[numThreads];
	
	srand(time(0)); // seed random number 
	// populate array with random numbers 
	#pragma omp parallel for num_threads(numThreads)
		for(int i = 0;  i < numElements; i++)
			arr[i] =  rand() % numElements + 1; // 1.. number elements  

	if(*flag == 'p') //  only print if passed p
	{
		cout << "Original list:" << endl;
		for(int i = 0; i < numElements; i++)
			cout << arr[i] << " ";    // printing elements of array 
		cout << endl << endl; 
	}
	start = omp_get_wtime(); // start time checking 
	#pragma omp parallel num_threads(numThreads) //  creating threads
	{
		int eleTotal, myFirst, myLast; //  number of elements in array,  starting index, ending index 
		int quotient = numElements / numThreads;
		int remainder = numElements % numThreads;
		int rank = omp_get_thread_num();  //  thread id
		// divide list, handles uneven case(provided by homework1)
		if(rank < remainder)
		{
			eleTotal = quotient + 1;
			myFirst =  rank * eleTotal;
		}
		else 
		{
			eleTotal = quotient;
			myFirst = rank * eleTotal + remainder;
		}
		
		chunks[rank] = eleTotal; //this rank is responsible for this many elements 		
		int *localList = new int[eleTotal]; // must be dynamically allocated 
											// not enough stack room 
		//copy to localList
		#pragma omp parallel for num_threads(numThreads) 
			for(int i = 0;  i < eleTotal; i++)
				localList[i] = arr[i+myFirst]; 
		
		if(*flag == 'p') // print only if flag is 'p'
		{
		#pragma omp critical // printing list shouldnt be interrupted
		{		
		cout << "thread_"<< rank << ", local_list: ";   //  prints locally sorted list 
		for(int i = 0; i < eleTotal; i++)
		{
			cout << localList[i] << " ";
		}
		cout << endl;
		}
		#pragma omp barrier // for printing purposes 
		if(rank == 0) cout << endl;
		}
		
		//sort localList
		qsort(localList, eleTotal, sizeof(int), compare);
		
		//copy  back to global array  
		#pragma omp parallel for num_threads(numThreads) 
			for(int i = 0;  i < eleTotal; i++)
				arr[i+myFirst] = localList[i]; 
		
		if(*flag == 'p') // print only if flag is 'p'
		{
		#pragma omp critical // printing list shouldnt be interrupted
		{		
		cout << "thread_"<< rank << ", sorted local_list: ";   //  prints locally sorted list 
		for(int i = 0; i < eleTotal; i++)
			cout << localList[i] << " ";
		cout << endl;
		}
		}
		
		int divisor =  2, coreDifference = 1; //tree reduction mechanism provided in homework 1
		while(divisor <= numThreads) //  while there are still sender/reciever pairs available
		{
			#pragma omp barrier   // all threads synchronize here
			if(rank % divisor == 0) // reciever thread determines partner and calls mergeort
			{
				int partner =  rank + coreDifference;
				merge(arr, myFirst, rank, partner);
			}
			divisor *= 2; coreDifference *= 2; 
		}
		end = omp_get_wtime(); //  stop time checking
		delete []localList; // free memory
	}	
	
	if(*flag == 'p') // print only if flag is 'p'
	{
	cout << "\nSorted List: " << endl; 
	for(int i = 0; i < numElements; i++)
		cout << arr[i] << " ";
	cout << endl; 
	}
	
 	(sorted(arr))? cout << "Verified  sorted \n" : cout << "Not sorted \n"; //  determine if list is sorted 
	cout <<"Using P=" << numThreads << ", n=" << numElements << ", Time elapsed: " << (end - start) << " sec" << endl; //  printing elasped time 
	
	delete []chunks; // free memory
	delete []arr; // free memory 
	return 0;
	}
	else 
		cout << "Invalid Command Line Arguments" << endl; // error checking 

}

/*
This function is responsible for merging lists back together after they that been individually
sorted using the quicksort function.
*/
int merge(int arr[],int start, int rank, int partRank)
{
	int partChunk, partFirst, partLast;
	int quotient = numElements / numThreads;
	int remainder = numElements % numThreads;
	if(partRank < remainder)
	{partChunk = quotient + 1; partFirst =  partRank * partChunk;}
	else 
	{partChunk = quotient; partFirst = partRank * partChunk + remainder;}
	
	int size = chunks[rank] + chunks[partRank]; //  size is both arrays combined (elements in one plus elements in other)
	int *temp = new int[size]; // not enough stack room,  must be dynamically allocated 
 	//............... Begin merge ................//
	int i = start; //  start of first array 
	int j = partFirst; // start of second array
	int k = 0; // index of temp array
		
	while(k < size) // while elements currently empty
	{
		if(arr[i] <= arr[j] && i < start + chunks[rank]) 
			temp[k++] = arr[i++]; // if i < j and there are elements in i
		else if( arr[j] < arr[i] && j < partFirst +chunks[partRank])
			temp[k++] = arr[j++];  // if j < i and there are elements in j
		else if(i >= start + chunks[rank])
			 temp[k++] = arr[j++]; // if no more i 
		else 
			 temp[k++] = arr[i++]; // if no more j
	}
	#pragma omp parallel for num_threads(numThreads)
		for(int i = 0;  i < size; i++)
			arr[i+start] = temp[i]; // copy back temp to global array 

	chunks[rank] += chunks[partRank]; // update number of elements rank is responsible for on next iteration
	delete []temp;
	}

// Utility function that checks if a list is sorted. 
//Checks to see if current element is smaller or equal to succeeding element. 
bool sorted(int arr[])
{
	for(int i=0;  i < numElements- 1;  i++)
	{
		if(arr[i] > arr[i+1])
			return false;  
	}
	return true; 
}
	
	
// This function is needed for quicksort
int compare(const void *aP ,  const void*bP)
{
	int a =  *((int*)aP); // cast to int*, then dereferenced  
	int b =  *((int*)bP);// cast to int*, then dereferenced 
	if (a < b) return -1; 
	else if (a == b) return 0;
	else return 1;  
}
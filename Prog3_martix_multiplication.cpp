/*
Adam Castillo
CS 176
In this programing assignment we were tasked with implementing a parallel
solution for matrix multipplication using pthreads. Rotational
assignment is  used to spread the work evenly among the threads. Thread_0 will
begin working at index zero and its next job will be  numthreads + 0. 
*/
#include <pthread.h>
#include <cstdlib>
#include <iostream>
#include <sys/time.h> // for GET_TIME

using namespace std;  
// Globals
int m,l,n, numthreads = 0;  
//dynamic 2D array implemented using double pointers
 long **a;
 long **b;
 long **c;
pthread_mutex_t print;
 // time macro provide by Dr. park
#define GET_TIME(now)\
{ struct timeval t; gettimeofday(&t, NULL);\
  now = t.tv_sec + t.tv_usec/1000000.0; } // microseconds(10^(-6)) converted to seconds
  										  // divide by 10^6
//prototype for slave function
void *matrixMulti(void *tid);

int main(int argc,  char** argv)
{
	// collect command line arguments 
	l =  atoi(argv[1]);
	m =  atoi(argv[2]);
	n =  atoi(argv[3]);
	numthreads = atoi(argv[4]);
	cout << "L= "<< l<< " M = " << m << " n = " <<  n << endl;  
	double start, stop; 
	// create 2D arrays
	a  =  new long*[l];
	for (int i = 0; i < l; ++i) 
 		 a[i] = new long[m];
 
 	b =  new long*[m];
 	for (int i = 0; i < m; ++i) 
  		b[i] = new long[n];

  	c = new long*[l];
  	for (int i = 0; i < l; ++i) 
  		c[i] = new long[n];

  	//populate A and B
  	for(int i = 0; i < l ; i ++)
  		for(int j = 0; j < m; j++)
  			a[i][j] = i*j+1; //  provided formula

  	 for(int i = 0; i < m ; i ++)
  		for(int j = 0; j < n; j++)
  			b[i][j] = i+j;	//provided formula
  	for(int i = 0; i < l ; i ++)
  		for(int j = 0; j < n; j++)
  			c[i][j] = 0;
  	pthread_t threads[numthreads]; //  data structure to hold the threads

  	GET_TIME(start); //  start timer
  	for(int i = 0; i < numthreads; i++) //  n number of threads
  	{ 
  		pthread_create(&threads[i], NULL, matrixMulti,(void*)i); // creating threads 
  	} 
  	for(int i = 0; i < numthreads;  i++)
  	{
  		pthread_join(threads[i], NULL); //wait for threads to complete before 
  										//  printing the resulting matrix
  	}
  	GET_TIME(stop); // stop timer 
  	// print first 20 
  	cout << endl << "====C: first_20*first_10===" << endl;  
  	for(int i = 0; i < 20;  i ++)
  	{
  		for(int j = 0; j < 10;  j++)
  		{
  			cout << c[i][j] << " ";
  		}
  		cout << endl;  
  	}

  	cout << endl << "====C: last_20*last_10===" << endl;  
  	for(int i = l-1; i > (l-1)-20;  i--) //  l-1 is last row 
  	{
  		for(int j = n-1; j > (n-1)-10;  j--) //  n-1 is last colmun 
  		{
  			cout << c[i][j] << " ";
  		}
  		cout << endl;  
  	}

  	cout << endl << "Time Taken(sec): " << stop-start << endl; 

	return 0; 
}

/*This function is responsible for calculating the matrix index given the 
thread id. This function evenly distributes the work by using rotation
assignment */ 
void *matrixMulti(void *tid)
{
	long my_tid = (long) tid; //convert parameter to long  
	long int  start = my_tid;
	// assign rows(l) using rotation way,
	// thread_0  start is 0, its next row is 0 + total number of threads
	//  which is tid * number threads

	cout << "Thread_"<< my_tid <<":" << my_tid << " ~ " << l - 1<< " step " << numthreads << endl;    
	for(int i = start; i < l; i += numthreads) //next row 
	{
		for(int j = 0;  j < n; j++)		 //  next column 
		{
			for(int k = 0; k < m;  k++) // iterate down row and column to
			{                           //calculate c[i][j]
			c[i][j] = c[i][j] + (a[i][k] * b[k][j]);
			} 
		}
	}
	pthread_exit(NULL);  // return nothing upon exit
}




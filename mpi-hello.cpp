////////////////////////////
//// Park - MPI hello program, C++ version
////   send: string type, receive: c_str type (static char array)
////
//// Compile/run:    
//// $> mpic++ -o hello1 mpi-hello.cpp
//// $> mpiexec -n 4 ./hello1   //4 is num of processes - any
//// Input: none
//// Output: A greeting from each process
////
//// Algorithm:  
////  Each process sends a message to process 0, which prints 
////  the messages it has received, as well as its own message.
//////////////////////////////

//#include <string.h> //for C_str strlen()
#include <mpi.h>      //For MPI functions, etc 
#include <iostream>
#include <string>
#include <sstream>
using namespace std;

const int MAX_STRING = 100;

int main(int argc, char *argv[]) 
{
   string greeting;  //to send string msg
   char greeting2[MAX_STRING]; //C-str to receive string msg
   int  comm_sz; //number of processes
   int  my_rank; //my process rank
   string comm_sz_str; //to convert int to string type
   string my_rank_str;

   MPI_Init(NULL, NULL); //start up MPI 
   MPI_Comm_size(MPI_COMM_WORLD, &comm_sz); //get num processes from -n x
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); //get my_rank among all

   //convert num_processes and my_rank to string type (to build a string msg) 
   stringstream out1, out2;
   out1<<comm_sz;
   comm_sz_str = out1.str();
   out2<<my_rank;
   my_rank_str = out2.str();

   if (my_rank != 0) //except proc_0, all send msg to proc_0
   { greeting = "Greetings from process_"+my_rank_str+" of "+comm_sz_str;
     //send message to process_0 (dest--4th para)
     MPI_Send(&greeting.at(0), greeting.length()+1, MPI_CHAR, 0, 0, MPI_COMM_WORLD); 
   } 
   else //if my_rank==0, receive msgs from other processes and display 
   { //print my message first
     cout<<"Greetings from process_"<<my_rank<<"! of "<<comm_sz<<endl;

     for (int q = 1; q < comm_sz; q++) 
     { //receive msg from process_q (source--4th para)
       MPI_Recv(greeting2, MAX_STRING, MPI_CHAR, q, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
       string greeting3 = greeting2; //for test purpose
       cout<<greeting3<<endl;        //also cout<<greeting2 works
      } 
   }//else

   MPI_Finalize(); //shut down MPI
   return 0;
}//main


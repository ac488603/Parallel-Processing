//////////////////////////////
//// Park-mpi-trap1.cpp    --C++ version
//// Purpose:  Use MPI to implement a parallel version of the trapezoid computation.
//// Input:    none -- a,b,n are statically defined in the program
//// Output:   Estimate of the integral from a to b of f(x)=x*x
////           using the trapezoidal rule and n trapezoids.
////
//// Compile:  mpic++ -o trap1 mpi-trap1.cpp 
//// Run:      mpiexec -n 4 ./trap1
////
//// Algorithm:
////  1.  Each process calculates "its" interval of integration.
////  2.  Each process estimates a local integral of f(x) over its interval.
////  3a. Each process != 0 sends its local integral to process_0.
////  3b. Process_0 sums the calculations received from all processes and prints.
////////////////////////////////

#include <cstdlib> //for exit(0)
#include <iostream>
#include <mpi.h>
using namespace std;

//Calculate local integral
double Trap(double left_endpt, double right_endpt, int trap_count, double base_len);    
//Function we're integrating, y=f(x)=x*x
double f(double x); 

int main(int argc, char* argv[]) 
{
  int my_rank, comm_sz;
  int n=1024, local_n;   
  double a=0.0, b=3.0, local_a, local_b, h;
  double local_int, total_int; //for local and global integral 
  int source; 

  MPI_Init(NULL, NULL); //start up MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); //get my process rank
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz); //get number of processes from (-n)

  h = (b-a)/n;         //h is same for all processes
  local_n = n/comm_sz; //number of trapezoids for each process

  local_a = a + my_rank*local_n*h; //local start
  local_b = local_a + local_n*h;   //local end
  local_int = Trap(local_a, local_b, local_n, h); //local_integral compute

  //add up the integrals calculated by each process
  if (my_rank != 0) 
  { cout<<"---in process_"<<my_rank<<", local_int="<<local_int<<endl;
    MPI_Send(&local_int, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD); //send to p_0 
  }
  else //process_0
  { cout<<"---in process_"<<my_rank<<", local_int="<<local_int<<endl;
    total_int = local_int; //initialize global_integral with p_0's local
    for (source = 1; source < comm_sz; source++) 
    { MPI_Recv(&local_int, 1, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      total_int += local_int; //update global_integral in process_0
    }
  }//else 

  if (my_rank == 0) //print result in process_0
    cout<<"With n="<<n<<" trapezoids, our estimate of integral from "<<a
        <<" to "<<b<<" is "<<total_int<<endl;     

  MPI_Finalize(); //shut down MPI
  return 0;
}//main

//////////////////////
//// Function:   Trap
//// Purpose:    Serial func for estimating a definite integral using trapezoidal rule
//// Input args: left_endpt, right_endpt, trap_count, base_len
//// Return val: Trapezoidal rule estimate of integral from left_endpt to right_endpt
////             using trap_count many trapezoids
//////////////////////
double Trap (double left_endpt, double right_endpt, int trap_count, double base_len)
{
  double estimate, x; 
  int i;

  estimate = (f(left_endpt) + f(right_endpt))/2.0;
  for (i = 1; i <= trap_count-1; i++) 
  { x = left_endpt + i*base_len;
    estimate += f(x);
  }
  estimate = estimate*base_len;
  return estimate;
}//Trap

/////////////////////
//// Compute value of function to be integrated
/////////////////////
double f(double x) 
{
  return x*x;
}//f


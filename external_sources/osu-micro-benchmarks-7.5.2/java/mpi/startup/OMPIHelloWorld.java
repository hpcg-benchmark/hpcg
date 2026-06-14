/**
* Copyright (c) 2002-2022 the Network-Based Computing Laboratory
* (NBCL), The Ohio State University.
* 
* Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
* 
* For detailed copyright and licensing information, please refer to the
* copyright file COPYRIGHT in the top level OMB directory.
*/

package mpi.startup;

import mpi.*;

public class OMPIHelloWorld {
    public static void main(String args[]) throws Exception {
        MPI.Init(args);
        int me = MPI.COMM_WORLD.getRank();
        int size = MPI.COMM_WORLD.getSize();
        System.out.println("Hi from <"+me+">");
        MPI.Finalize();
    }
} 

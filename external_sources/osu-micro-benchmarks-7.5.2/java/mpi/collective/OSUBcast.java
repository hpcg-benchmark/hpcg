/**
* Copyright (c) 2002-2022 the Network-Based Computing Laboratory
* (NBCL), The Ohio State University.
* 
* Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
* 
* For detailed copyright and licensing information, please refer to the
* copyright file COPYRIGHT in the top level OMB directory.
*/

package mpi.collective;

import mpi.common.BenchmarkUtils;
import mpi.*;
import java.nio.ByteBuffer;
import java.io.* ;

public class OSUBcast extends BenchmarkUtils {

  private String[] args = null;
  Object buffer = null;
  int me;

  public OSUBcast() {
  }

  public OSUBcast(String[] args) {
    super(args, BenchmarkUtils.OSU_BCAST_TEST);
    this.args = args;
  }
 
  public void runBenchmark() throws Exception {

    double latency = 0L;
    long start = 0L, stop = 0L;
    double init = 0.0;
    double totalTime = 0.0;

    me = MPI.COMM_WORLD.getRank(); 

    int size = 1, i = 0;   	       		   	
    long timed = 0L;		
    int root = 0 ;

    if(useBufferAPI) {
      buffer = ByteBuffer.allocateDirect(maxMsgSize);
    } else {
      buffer = new byte[maxMsgSize];
    }

    runWarmupLoop();

    for (size = minMsgSize ; size <= maxMsgSize ; size = 2*size) {

      if (size > LARGE_MESSAGE_SIZE) {
        benchIters = COLL_LOOP_LARGE;
        benchWarmupIters = COLL_SKIP_LARGE;
      }

      for (i = 0 ; i < benchWarmupIters + benchIters ; i++) {

        totalTime = 0.0;

        if(i == benchWarmupIters) {
          init = System.nanoTime();
        }

        if (validateData) {

          if(useBufferAPI) {
            if(me == root) {
              fillData((ByteBuffer)buffer, ((byte)1), size);
            } else {     
              fillData((ByteBuffer)buffer, ((byte)0), size);
            } 
          } else {
            if(me == root) {
              fillData((byte[])buffer, ((byte)1), size);
            } else { 
              fillData((byte[])buffer, ((byte)0), size);
            } 
          } 

        }

        if(useBufferAPI) {
          MPI.COMM_WORLD.bcast((ByteBuffer)buffer, size, MPI.BYTE, root);
        } else { 
          MPI.COMM_WORLD.bcast((byte[])buffer, size, MPI.BYTE, root);
        } 

        if (validateData) {
          
          if(useBufferAPI) {
            if(me == root) {
              validateDataAfterSend((ByteBuffer)buffer, size);
            } else {     
              validateDataAfterRecv((ByteBuffer)buffer, size);
            } 
          } else {
            if(me == root) {
              validateDataAfterSend((byte[])buffer, size);
            } else { 
              validateDataAfterRecv((byte[])buffer, size);
            }
          }

        }

        if(i == (benchWarmupIters + benchIters -1)) {
          totalTime += (System.nanoTime() - init) / (1E9*1.0);
        }

      } //end benchmarking loop

      double latencyToPrint = (totalTime * 1e6) / benchIters;

      printStats(latencyToPrint, size);

      MPI.COMM_WORLD.barrier() ;
    }

  }

  private void printStats(double lat, int size) throws Exception {

    double[] latencyIn = { lat };
    double[] latencyAvg = { 0 };
    double[] latencyMin = { 0 };
    double[] latencyMax = { 0 };

    MPI.COMM_WORLD.reduce(latencyIn, latencyMin, 1, MPI.DOUBLE, MPI.MIN, 0);
    MPI.COMM_WORLD.reduce(latencyIn, latencyAvg, 1, MPI.DOUBLE, MPI.SUM, 0);
    MPI.COMM_WORLD.reduce(latencyIn, latencyMax, 1, MPI.DOUBLE, MPI.MAX, 0);

    latencyAvg[0] = latencyAvg[0] / MPI.COMM_WORLD.getSize();

    if(MPI.COMM_WORLD.getRank() == 0) { 
      System.out.println(size + "\t\t" + 
          String.format("%.2f", latencyAvg[0]) + "\t\t\t" + 
          String.format("%.2f", latencyMin[0]) + "\t\t\t" +
          String.format("%.2f", latencyMax[0]));
    }

  }

  private void runWarmupLoop() throws Exception {

    /* Warmup Loop */
    for(int i=0 ; i<INITIAL_WARMUP; i++) {

      if(useBufferAPI) {
        MPI.COMM_WORLD.bcast((ByteBuffer)buffer, 1024, MPI.BYTE, 0);
      } else { 
        MPI.COMM_WORLD.bcast((byte[])buffer, 1024, MPI.BYTE, 0);
      } 

      MPI.COMM_WORLD.barrier();

    } //end benchWarmupIters loop */
  }

  protected void printHeader() {
    System.out.println(OSU_BCAST_TEST);
    System.out.println("# Size" + "\t\t" + "Lat Avg[us]" + "\t\t" + 
        "Lat Min[us]"+ "\t\t" + "Lat Max[us]"); 
        
  }
  
  public static void main(String[] args) throws Exception {
    OSUBcast bcastTest = new OSUBcast(args);

    MPI.Init(args); 

    if (MPI.COMM_WORLD.getRank() == 0)
      bcastTest.printHeader();

    if (bcastTest.printVersion) {
      if (MPI.COMM_WORLD.getRank() == 0)
        bcastTest.printVersion();

      MPI.Finalize();
      return;
    }

    if (bcastTest.printHelp) {
      if (MPI.COMM_WORLD.getRank() == 0)
        bcastTest.printHelp();

      MPI.Finalize();
      return;
    }

    if(MPI.COMM_WORLD.getSize() < 2) {
      if (MPI.COMM_WORLD.getRank() == 0) {
        System.out.println("This test requires at least two processes");
      }

      MPI.Finalize();
      return;
    }

    bcastTest.runBenchmark();

    MPI.Finalize();

  }
  
}

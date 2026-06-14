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

public class OSUBarrier extends BenchmarkUtils {

  private String[] args = null;
  int me;

  public OSUBarrier() {
  }

  public OSUBarrier(String[] args) {
    super(args, BenchmarkUtils.OSU_BARRIER_TEST);
    this.args = args;
  }
 
  public void runBenchmark() throws Exception {

    double latency = 0L;
    long start = 0L, stop = 0L;
    long init = 0L;
    double totalTime = 0.0;

    me = MPI.COMM_WORLD.getRank(); 

    int size = 1, i = 0;   	       		   	
    long timed = 0L;		

    runWarmupLoop();

    for (i = 0 ; i < benchWarmupIters + benchIters ; i++) {

      if(i == benchWarmupIters) {
        init = System.nanoTime(); 
      }

      MPI.COMM_WORLD.barrier();

      if(i == (benchWarmupIters + benchIters -1)) {
        totalTime += (System.nanoTime() - init) / (1E9*1.0);
      }

    }

    double latencyToPrint = (totalTime * 1e6) / benchIters;

    printStats(latencyToPrint);
  }

  private void printStats(double lat) throws Exception {

    double[] latencyIn = { lat };
    double[] latencyAvg = { 0 };
    double[] latencyMin = { 0 };
    double[] latencyMax = { 0 };

    MPI.COMM_WORLD.reduce(latencyIn, latencyMin, 1, MPI.DOUBLE, MPI.MIN, 0);
    MPI.COMM_WORLD.reduce(latencyIn, latencyAvg, 1, MPI.DOUBLE, MPI.SUM, 0);
    MPI.COMM_WORLD.reduce(latencyIn, latencyMax, 1, MPI.DOUBLE, MPI.MAX, 0);

    latencyAvg[0] = latencyAvg[0] / MPI.COMM_WORLD.getSize();

    if(MPI.COMM_WORLD.getRank() == 0) { 
      System.out.println("  " + String.format("%.2f", latencyAvg[0]) + "\t\t\t\t" + 
          String.format("%.2f", latencyMin[0]) + "\t\t\t\t" +
          String.format("%.2f", latencyMax[0]));
    }

  }

  private void runWarmupLoop() throws Exception {

    /* Warmup Loop */
    for(int i=0 ; i<INITIAL_WARMUP; i++) {

      MPI.COMM_WORLD.barrier();

    } //end benchWarmupIters loop */
  }

  protected void printHeader() {
    System.out.println(OSU_BARRIER_TEST);
    System.out.println("# Avg. Latency [us]" + "\t\t" + "Min. Latency [us]"+ "\t\t" +
        "Max. Latency [us]"); 
  }
  
  public static void main(String[] args) throws Exception {
    OSUBarrier barrierTest = new OSUBarrier(args);

    MPI.Init(args); 

    if (MPI.COMM_WORLD.getRank() == 0)
      barrierTest.printHeader();

    if (barrierTest.printVersion) {
      if (MPI.COMM_WORLD.getRank() == 0)
        barrierTest.printVersion();

      MPI.Finalize();
      return;
    }

    if (barrierTest.printHelp) {
      if (MPI.COMM_WORLD.getRank() == 0)
        barrierTest.printHelp();

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

    barrierTest.runBenchmark();

    MPI.Finalize();

  }
  
}

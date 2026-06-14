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
import java.nio.Buffer;

import java.nio.ByteOrder;

import java.io.* ;

public class OSUReduceScatter extends BenchmarkUtils {

  private String[] args = null;
  Object sendBuffer = null;
  Object recvBuffer = null;
  int me;
  int numprocs;

  public OSUReduceScatter() {
  }

  public OSUReduceScatter(String[] args) {
    super(args, BenchmarkUtils.OSU_REDUCESCATTER_TEST);
    this.args = args;
  }
 
  public void runBenchmark() throws Exception {

    double latency = 0L;
    long start = 0L, stop = 0L;
    double init = 0.0;
    double totalTime = 0.0;

    me = MPI.COMM_WORLD.getRank(); 
    numprocs = MPI.COMM_WORLD.getSize();

    int size = 1, i = 0;   	       	
    long timed = 0L;		
    int root = 0 ;
    int[] recvcounts = new int[numprocs];

    if(useBufferAPI) {
      sendBuffer = ByteBuffer.allocateDirect(maxMsgSize);
      recvBuffer = ByteBuffer.allocateDirect(
          FLOAT_SIZE * (maxMsgSize / numprocs / FLOAT_SIZE + 1));
      ((ByteBuffer)sendBuffer).order(ByteOrder.LITTLE_ENDIAN);
      ((ByteBuffer)recvBuffer).order(ByteOrder.LITTLE_ENDIAN);
    } else {
      sendBuffer = new float[maxMsgSize];
      recvBuffer = 
          new float[FLOAT_SIZE * (maxMsgSize / numprocs / FLOAT_SIZE + 1)];
    }

    runWarmupLoop();

    for (size = minMsgSize ; size*FLOAT_SIZE <= maxMsgSize ; size = 2*size) {

      if (size > LARGE_MESSAGE_SIZE) {
        benchIters = COLL_LOOP_LARGE;
        benchWarmupIters = COLL_SKIP_LARGE;
      }

      int portion = 0, remainder = 0;
      portion = size / numprocs;
      remainder = size % numprocs;

      for (i = 0; i < numprocs; i++) {
        recvcounts[i] = 0;

        if(size < numprocs) {
          if(i < size)
            recvcounts[i] = 1;
        } else {
          if((remainder != 0) && (i < remainder)) {
            recvcounts[i] += 1;
          }
          recvcounts[i] += portion;
        }
      }

      for (i = 0 ; i < benchWarmupIters + benchIters ; i++) {

        totalTime = 0.0;

        if(i == benchWarmupIters) {
          init = System.nanoTime();
        }

        if (validateData) {
          if(useBufferAPI) {
              fillSendAndRecvBufferForReduceScatter((ByteBuffer)sendBuffer, 
                  (ByteBuffer)recvBuffer, size, recvcounts[me]);
          } else {
              fillSendAndRecvBufferForReduceScatter((float[])sendBuffer, 
                  (float[])recvBuffer, size, recvcounts[me]);
          }
        }

        if(useBufferAPI) {
          ((ByteBuffer)sendBuffer).clear();
          ((ByteBuffer)recvBuffer).clear();
        }

        if(useBufferAPI) {
          MPI.COMM_WORLD.reduceScatter(sendBuffer, recvBuffer, recvcounts, 
              MPI.FLOAT, MPI.SUM);
        } else { 
          MPI.COMM_WORLD.reduceScatter(sendBuffer, recvBuffer, recvcounts, 
              MPI.FLOAT, MPI.SUM);
        }

        if (validateData && me == 0) {
          if(useBufferAPI) {
            validateDataAfterReduce((ByteBuffer)sendBuffer, 
                (ByteBuffer)recvBuffer, recvcounts[me], MPI.COMM_WORLD.getSize());
          } else {
            validateDataAfterReduce((float[])sendBuffer, 
                (float[])recvBuffer, recvcounts[me], MPI.COMM_WORLD.getSize());
          }
        }

        if(useBufferAPI) {
          ((ByteBuffer)sendBuffer).clear();
          ((ByteBuffer)recvBuffer).clear();
        } 

        if(i == (benchWarmupIters + benchIters -1)) {
          totalTime += (System.nanoTime() - init) / (1E9*1.0);
        }

      } //end benchmarking loop

      double latencyToPrint = (totalTime * 1e6) / benchIters;

      printStats(latencyToPrint, size * FLOAT_SIZE);

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

    int[] recvcounts = new int[numprocs];

    for (int i = 0; i < numprocs; i++)
      recvcounts[i] = 1024;

    /* Warmup Loop */
    for(int i=0 ; i<INITIAL_WARMUP; i++) {

      MPI.COMM_WORLD.reduceScatter(sendBuffer, recvBuffer, recvcounts, 
          MPI.FLOAT, MPI.SUM);

      MPI.COMM_WORLD.barrier();

    } //end benchWarmupIters loop
  }

  protected void printHeader() {
    System.out.println(OSU_REDUCESCATTER_TEST);
    System.out.println("# Size" + "\t\t" + "Lat Avg[us]" + "\t\t" + 
        "Lat Min[us]"+ "\t\t" + "Lat Max[us]"); 
        
  }
  
  public static void main(String[] args) throws Exception {
    OSUReduceScatter reduceScatterTest = new OSUReduceScatter(args);

    MPI.Init(args); 

    if (MPI.COMM_WORLD.getRank() == 0)
      reduceScatterTest.printHeader();

    if (reduceScatterTest.printVersion) {
      if (MPI.COMM_WORLD.getRank() == 0)
        reduceScatterTest.printVersion();

      MPI.Finalize();
      return;
    }

    if (reduceScatterTest.printHelp) {
      if (MPI.COMM_WORLD.getRank() == 0)
        reduceScatterTest.printHelp();

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

    reduceScatterTest.runBenchmark();

    MPI.Finalize();

  }
  
}

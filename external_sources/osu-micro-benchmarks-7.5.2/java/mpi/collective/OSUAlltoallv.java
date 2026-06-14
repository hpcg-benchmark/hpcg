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

import java.io.*;

public class OSUAlltoallv extends BenchmarkUtils {

  private String[] args = null;
  Object sendBuffer = null;
  Object recvBuffer = null;
  int me;
  int tasks;
  int[] sdispls;
  int[] sendcounts;
  int[] rdispls;
  int[] recvcounts;

  public OSUAlltoallv() {
  }

  public OSUAlltoallv(String[] args) {
    super(args, BenchmarkUtils.OSU_ALLTOALLV_TEST);
    this.args = args;
  }
 
  public void runBenchmark() throws Exception {

    double latency = 0L;
    long start = 0L, stop = 0L;
    double init = 0.0;
    double totalTime = 0.0;

    me = MPI.COMM_WORLD.getRank(); 
    tasks = MPI.COMM_WORLD.getSize(); 

    int size = 1, i = 0;   	       		   	
    long timed = 0L;		
    int root = 0 ;
    int disp;
 
    sdispls = new int[tasks];
    sendcounts = new int[tasks];
    rdispls = new int[tasks];
    recvcounts = new int[tasks];

    if(useBufferAPI) {
      sendBuffer = ByteBuffer.allocateDirect(maxMsgSize * tasks);
      recvBuffer = ByteBuffer.allocateDirect(maxMsgSize * tasks);
      ((ByteBuffer)sendBuffer).order(ByteOrder.LITTLE_ENDIAN);
      ((ByteBuffer)recvBuffer).order(ByteOrder.LITTLE_ENDIAN);
    } else {
      sendBuffer = new byte[maxMsgSize * tasks];
      recvBuffer = new byte[maxMsgSize * tasks];
    }

    runWarmupLoop();

    for (size = minMsgSize ; size <= maxMsgSize ; size = 2*size) {

      if (size > LARGE_MESSAGE_SIZE) {
        benchIters = COLL_LOOP_LARGE;
        benchWarmupIters = COLL_SKIP_LARGE;
      }

      disp = 0;

      for ( i = 0; i < tasks; i++) {
        sendcounts[i] = size;
        sdispls[i] = disp;
        recvcounts[i] = size;
        rdispls[i] = disp;
        disp += size;
      }

      for (i = 0 ; i < benchWarmupIters + benchIters ; i++) {

        totalTime = 0.0;

        if(i == benchWarmupIters) {
          init = System.nanoTime();
        }

        if (validateData) {
          if(useBufferAPI) {
            fillData((ByteBuffer)sendBuffer, (byte)1, size * tasks);
            fillData((ByteBuffer)recvBuffer, (byte)0, size * tasks);
          } else {
            fillData((byte[])sendBuffer, (byte)1, size * tasks);
            fillData((byte[])recvBuffer, (byte)0, size * tasks);
          }
        }

        if(useBufferAPI) {
          MPI.COMM_WORLD.allToAllv(sendBuffer, sendcounts, sdispls, MPI.BYTE, 
              recvBuffer, recvcounts, rdispls, MPI.BYTE);
        } else { 
          MPI.COMM_WORLD.allToAllv(sendBuffer, sendcounts, sdispls, MPI.BYTE, 
              recvBuffer, recvcounts, rdispls, MPI.BYTE);
        }

        if (validateData && MPI.COMM_WORLD.getRank() == root) {
          if(useBufferAPI) {
            validateDataAfterSend((ByteBuffer)sendBuffer, size * tasks);
            validateDataAfterRecv((ByteBuffer)recvBuffer, size * tasks);
          } else {
            validateDataAfterSend((byte[])sendBuffer, size * tasks);
            validateDataAfterRecv((byte[])recvBuffer, size * tasks);
          }

        } else { 
          if(useBufferAPI) {
            //clear sendBuffer for non-root procs
            ((ByteBuffer)sendBuffer).clear(); 
            ((ByteBuffer)recvBuffer).clear(); 
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

      for ( int j = 0; j < tasks; j++) {
        sendcounts[j] = 1024;
        sdispls[j] = 1024 * j;
        recvcounts[j] = 1024;
        rdispls[j] = 1024 * j;
      }

      if(useBufferAPI) {
        MPI.COMM_WORLD.allToAllv(sendBuffer, sendcounts, sdispls, MPI.BYTE, 
            recvBuffer, recvcounts, rdispls, MPI.BYTE);
      } else { 
        MPI.COMM_WORLD.allToAllv(sendBuffer, sendcounts, sdispls, MPI.BYTE, 
            recvBuffer, recvcounts, rdispls, MPI.BYTE);
      }

      MPI.COMM_WORLD.barrier();

    } //end benchWarmupIters loop
  }

  protected void printHeader() {
    System.out.println(OSU_ALLTOALLV_TEST);
    System.out.println("# Size" + "\t\t" + "Lat Avg[us]" + "\t\t" + 
        "Lat Min[us]"+ "\t\t" + "Lat Max[us]"); 
        
  }
  
  public static void main(String[] args) throws Exception {
    OSUAlltoallv alltoallvTest = new OSUAlltoallv(args);

    MPI.Init(args); 

    if (MPI.COMM_WORLD.getRank() == 0)
      alltoallvTest.printHeader();

    if (alltoallvTest.printVersion) {
      if (MPI.COMM_WORLD.getRank() == 0)
        alltoallvTest.printVersion();

      MPI.Finalize();
      return;
    }

    if (alltoallvTest.printHelp) {
      if (MPI.COMM_WORLD.getRank() == 0)
        alltoallvTest.printHelp();

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

    alltoallvTest.runBenchmark();

    MPI.Finalize();

  }
  
}

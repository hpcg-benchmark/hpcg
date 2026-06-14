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

public class OSUScatterv extends BenchmarkUtils {

  private String[] args = null;
  Object sendBuffer = null;
  Object recvBuffer = null;
  int me;
  int tasks;
  int[] sdispls;
  int[] sendcounts;

  public OSUScatterv() {
  }

  public OSUScatterv(String[] args) {
    super(args, BenchmarkUtils.OSU_SCATTERV_TEST);
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

    if(useBufferAPI) {
      sendBuffer = ByteBuffer.allocateDirect(maxMsgSize * tasks);
      recvBuffer = ByteBuffer.allocateDirect(maxMsgSize);
      ((ByteBuffer)sendBuffer).order(ByteOrder.LITTLE_ENDIAN);
      ((ByteBuffer)recvBuffer).order(ByteOrder.LITTLE_ENDIAN);
    } else {
      sendBuffer = new byte[maxMsgSize * tasks];
      recvBuffer = new byte[maxMsgSize];
    }

    runWarmupLoop();
    //benchIters = 10;
    //benchWarmupIters = 10;
    //maxMsgSize = 1024;

    for (size = minMsgSize ; size <= maxMsgSize ; size = 2*size) {

      if (size > LARGE_MESSAGE_SIZE) {
        benchIters = COLL_LOOP_LARGE;
        benchWarmupIters = COLL_SKIP_LARGE;
      }

      if (me == 0) {
        disp = 0;
        for ( i = 0; i < tasks; i++) {
          sendcounts[i] = size;
          sdispls[i] = disp;
          disp += size;
        }
      }

      for (i = 0 ; i < benchWarmupIters + benchIters ; i++) {

        totalTime = 0.0;

        if(i == benchWarmupIters) {
          init = System.nanoTime();
        }

        if (validateData) {
          if(useBufferAPI) {
            fillData((ByteBuffer)sendBuffer, (byte)1, size * tasks);
            fillData((ByteBuffer)recvBuffer, (byte)0, size);
          } else {
            fillData((byte[])sendBuffer, (byte)1, size * tasks);
            fillData((byte[])recvBuffer, (byte)0, size);
          }
        }

        if(useBufferAPI) {
          MPI.COMM_WORLD.scatterv(sendBuffer, sendcounts, sdispls, MPI.BYTE, 
              recvBuffer, size, MPI.BYTE, 0);
        } else { 
          MPI.COMM_WORLD.scatterv(sendBuffer, sendcounts, sdispls, MPI.BYTE, 
              recvBuffer, size, MPI.BYTE, 0);
        }

        if (validateData && MPI.COMM_WORLD.getRank() == root) {
          if(useBufferAPI) {
            validateDataAfterSend((ByteBuffer)sendBuffer, size * tasks);
            validateDataAfterRecv((ByteBuffer)recvBuffer, size);
          } else {
            validateDataAfterSend((byte[])sendBuffer, size * tasks);
            validateDataAfterRecv((byte[])recvBuffer, size);
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
      }

      if(useBufferAPI) {
        MPI.COMM_WORLD.scatterv(sendBuffer, sendcounts, sdispls, MPI.BYTE, 
            recvBuffer, 1024, MPI.BYTE, 0);
      } else { 
        MPI.COMM_WORLD.scatterv(sendBuffer, sendcounts, sdispls, MPI.BYTE, 
            recvBuffer, 1024, MPI.BYTE, 0);
      }

      MPI.COMM_WORLD.barrier();

    } //end benchWarmupIters loop
  }

  protected void printHeader() {
    System.out.println(OSU_SCATTERV_TEST);
    System.out.println("# Size" + "\t\t" + "Lat Avg[us]" + "\t\t" + 
        "Lat Min[us]"+ "\t\t" + "Lat Max[us]"); 
        
  }
  
  public static void main(String[] args) throws Exception {
    OSUScatterv scattervTest = new OSUScatterv(args);

    MPI.Init(args); 

    if (MPI.COMM_WORLD.getRank() == 0)
      scattervTest.printHeader();

    if (scattervTest.printVersion) {
      if (MPI.COMM_WORLD.getRank() == 0)
        scattervTest.printVersion();

      MPI.Finalize();
      return;
    }

    if (scattervTest.printHelp) {
      if (MPI.COMM_WORLD.getRank() == 0)
        scattervTest.printHelp();

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

    scattervTest.runBenchmark();

    MPI.Finalize();

  }
  
}

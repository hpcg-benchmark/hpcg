/**
* Copyright (c) 2002-2022 the Network-Based Computing Laboratory
* (NBCL), The Ohio State University.
* 
* Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
* 
* For detailed copyright and licensing information, please refer to the
* copyright file COPYRIGHT in the top level OMB directory.
*/

package mpi.pt2pt;

import mpi.common.BenchmarkUtils;
import mpi.*;
import java.nio.ByteBuffer;
import java.io.* ;

public class OSULatency extends BenchmarkUtils {

  private String[] args = null;
  Object sendBuffer = null; 
  Object recvBuffer = null;
  int me;

  public OSULatency() {
  }

  public OSULatency(String[] args) {
    super(args, BenchmarkUtils.OSU_LAT_TEST);
    this.args = args;
  }
 
  public void runBenchmark() throws Exception {

    double latency = 0L;
    long start = 0L, stop = 0L, init = 0L;

    me = MPI.COMM_WORLD.getRank(); 
    System.out.println("Proc <" + me + "> on <" + MPI.getProcessorName() +">");

    int size = 1, i = 0;   	       		   	
    long timed = 0L;		


    if(useBufferAPI) {
      sendBuffer = ByteBuffer.allocateDirect(maxMsgSize);
      recvBuffer = ByteBuffer.allocateDirect(maxMsgSize);
    } else {
      sendBuffer = new byte[maxMsgSize];
      recvBuffer = new byte[maxMsgSize];
    }

    runWarmupLoop();

    /* Benchmarking Loop */
    for (size = minMsgSize ; size <= maxMsgSize ; size = 2*size) {

        if (size > LARGE_MESSAGE_SIZE) {
          benchIters = LAT_LOOP_LARGE;
          benchWarmupIters = LAT_SKIP_LARGE;
        }

        for (i = 0 ; i < benchWarmupIters + benchIters ; i++) {

	  if(me == 0) {

            if(i == benchWarmupIters) {
	      init = System.nanoTime();
            }

            if (validateData) {
              if(useBufferAPI)
                fillData((ByteBuffer)sendBuffer, (ByteBuffer)recvBuffer, size);
              else 
                fillData((byte[])sendBuffer, (byte[])recvBuffer, size);
            }

            if(useBufferAPI)
              MPI.COMM_WORLD.send((ByteBuffer)sendBuffer, size, MPI.BYTE, 1, 998);
            else
              MPI.COMM_WORLD.send((byte[])sendBuffer, size, MPI.BYTE, 1, 998);

            if (validateData) {
              if(useBufferAPI)
                validateDataAfterSend((ByteBuffer)sendBuffer, 
                    (ByteBuffer)recvBuffer, size);
              else 
                validateDataAfterSend((byte[])sendBuffer, (byte[])recvBuffer, size);
            }

            if (validateData) {
              if(useBufferAPI)
                fillData((ByteBuffer)sendBuffer, (ByteBuffer)recvBuffer, size);
              else
                fillData((byte[])sendBuffer, (byte[])recvBuffer, size);
            }

            if(useBufferAPI)
                MPI.COMM_WORLD.recv(
                    (ByteBuffer)recvBuffer, size, MPI.BYTE, 1, 998);
            else
                MPI.COMM_WORLD.recv(
                    (byte[])recvBuffer, size, MPI.BYTE, 1, 998);

            if (validateData) {
              if(useBufferAPI)
                validateDataAfterRecv((ByteBuffer)sendBuffer, 
                    (ByteBuffer)recvBuffer, size);
              else
                validateDataAfterRecv((byte[])sendBuffer, (byte[])recvBuffer, size);
            }

            if(i == (benchWarmupIters + benchIters -1)) {
	      latency = (System.nanoTime() - init)/(2.0*benchIters*1000) ; 
            }

	  } else if(me == 1) {

            if (validateData) {
              if(useBufferAPI)
                fillData((ByteBuffer)sendBuffer, (ByteBuffer)recvBuffer, size);
              else
                fillData((byte[])sendBuffer, (byte[])recvBuffer, size);
            }

            if(useBufferAPI)
              MPI.COMM_WORLD.recv(
                  (ByteBuffer)recvBuffer, size, MPI.BYTE, 0, 998);
            else 
              MPI.COMM_WORLD.recv(
                  (byte[])recvBuffer, size, MPI.BYTE, 0, 998);

            if (validateData) {
              if(useBufferAPI)
                validateDataAfterRecv((ByteBuffer)sendBuffer, 
                    (ByteBuffer)recvBuffer, size);
              else
                validateDataAfterRecv((byte[])sendBuffer, (byte[])recvBuffer, size);
            }

            if (validateData) {
              if(useBufferAPI)
                fillData((ByteBuffer)sendBuffer, (ByteBuffer)recvBuffer, size);
              else
                fillData((byte[])sendBuffer, (byte[])recvBuffer, size);
            }

            if(useBufferAPI)
              MPI.COMM_WORLD.send((ByteBuffer)sendBuffer, size, MPI.BYTE, 0, 998);
             else
              MPI.COMM_WORLD.send((byte[])sendBuffer, size, MPI.BYTE, 0, 998);			
            if (validateData) {
              if(useBufferAPI)
                validateDataAfterSend((ByteBuffer)sendBuffer, 
                    (ByteBuffer)recvBuffer, size);
              else
                validateDataAfterSend((byte[])sendBuffer, (byte[])recvBuffer, size);
            }

	  }

	}

        double bandwidthInGbps = (( 8*size ) /( 1000* (latency)));
	if(me == 0) {
          System.out.println(size + "\t\t\t" + 
              String.format("%.2f", latency) + "\t\t\t" +
              String.format("%.2f", bandwidthInGbps));
	}

        MPI.COMM_WORLD.barrier() ;   

    } //end benchmarking loop

  }

  private void runWarmupLoop() throws Exception {

    /* Warmup Loop */
    for(int i=0 ; i<INITIAL_WARMUP; i++) {

      if(me == 0) {

        if(useBufferAPI) {
          MPI.COMM_WORLD.recv((ByteBuffer)recvBuffer, 1024, MPI.BYTE, 1, 998);
          MPI.COMM_WORLD.send((ByteBuffer)sendBuffer, 1024, MPI.BYTE, 1, 998);
        } else {
          MPI.COMM_WORLD.recv((byte[])recvBuffer, 1024, MPI.BYTE, 1, 998);
          MPI.COMM_WORLD.send((byte[])sendBuffer, 1024, MPI.BYTE, 1, 998);
        }

      } else if(me == 1) {

        if(useBufferAPI) {
          MPI.COMM_WORLD.send((ByteBuffer)sendBuffer, 1024, MPI.BYTE, 0, 998);
          MPI.COMM_WORLD.recv((ByteBuffer)recvBuffer, 1024, MPI.BYTE, 0, 998);
        } else {
          MPI.COMM_WORLD.send((byte[])sendBuffer, 1024, MPI.BYTE, 0, 998);
          MPI.COMM_WORLD.recv((byte[])recvBuffer, 1024, MPI.BYTE, 0, 998);
        }

      }

    } //end benchWarmupIters loop
  }

  protected void printHeader() {
    System.out.println(OSU_LAT_TEST);
    System.out.println("# Size [B]" + "\t\t" + "Latency [us]" + "\t" +
        "Bandwidth [Gb/s]");
  }
   
  public static void main(String[] args) throws Exception {

    OSULatency latencyTest = new OSULatency(args);

    MPI.Init(args);

    if (MPI.COMM_WORLD.getRank() == 0)
      latencyTest.printHeader();

    if (latencyTest.printVersion) {
      if (MPI.COMM_WORLD.getRank() == 0)
        latencyTest.printVersion();

      MPI.Finalize();
      return;
    }

    if (latencyTest.printHelp) {
      if (MPI.COMM_WORLD.getRank() == 0)
        latencyTest.printHelp();

      MPI.Finalize();
      return;
    }

    latencyTest.runBenchmark();

    MPI.Finalize();

  }
  
}

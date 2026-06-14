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

public class OSUBandwidth extends BenchmarkUtils {

  private String[] args = null;
  Object[] sendBuffer = null;
  Object[] recvBuffer = null;
  int me;

  public OSUBandwidth() {

  }

  public OSUBandwidth(String[] args) {
    super(args, BenchmarkUtils.OSU_BW_TEST);
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

    sendBuffer = new Object[windowSize]; 
    recvBuffer = new Object[windowSize];

    for(int winIdx=0 ; winIdx<windowSize ; winIdx++) {
      if(useBufferAPI) {
        sendBuffer[winIdx] = ByteBuffer.allocateDirect(maxMsgSize);
        recvBuffer[winIdx] = ByteBuffer.allocateDirect(maxMsgSize);
      } else {
        sendBuffer[winIdx] = new byte[maxMsgSize];
        recvBuffer[winIdx] = new byte[maxMsgSize];
      }
    }

    Request[] reqs = new Request[windowSize];

    /* Benchmarking Loop */
    for (size = minMsgSize ; size <= maxMsgSize ; size = 2*size) {

        totalTime = 0.0;

        if (size >= MESSAGE_SIZE_ONE_MB) {
          benchIters = BW_LOOP_LARGE_ONE_MB;
          benchWarmupIters = BW_SKIP_LARGE_ONE_MB;
        } else if (size > LARGE_MESSAGE_SIZE) {
          benchIters = BW_LOOP_LARGE; 
          benchWarmupIters = BW_SKIP_LARGE;
        }

	for (i = 0 ; i < benchWarmupIters + benchIters ; i++) {

	  if(me == 0) {

            if(i == benchWarmupIters) {
	      init = System.nanoTime(); 
            }

            if (validateData) {
              if(useBufferAPI) {
                for (int winIdx = 0 ; winIdx < windowSize ; winIdx++) 
                  fillData((ByteBuffer)sendBuffer[winIdx], 
                      (ByteBuffer)recvBuffer[winIdx], size);
              } else {
                for (int winIdx = 0 ; winIdx < windowSize ; winIdx++) 
                  fillData((byte[])sendBuffer[winIdx], 
                      (byte[])recvBuffer[winIdx], size);
              }
            }

            if(useBufferAPI) {
              for (int winIdx = 0 ; winIdx < windowSize ; winIdx++) {
                reqs[winIdx] = MPI.COMM_WORLD.iSend(
                    (ByteBuffer)sendBuffer[winIdx], size, MPI.BYTE, 1, 100);
              } 
            } else {
              for (int winIdx = 0 ; winIdx < windowSize ; winIdx++) {
                reqs[winIdx] = MPI.COMM_WORLD.iSend(
                    (byte[])(sendBuffer[winIdx]), size, MPI.BYTE, 1, 100);
              } 
            }

            Request.waitAllStatus(reqs);

            if (validateData) {
              if(useBufferAPI) {
                for (int winIdx = 0 ; winIdx < windowSize ; winIdx++) {
                  validateDataAfterSend((ByteBuffer)sendBuffer[winIdx], 
                      (ByteBuffer)recvBuffer[winIdx], size);
                }
              } else {
                for (int winIdx = 0 ; winIdx < windowSize ; winIdx++) 
                  validateDataAfterSend((byte[])sendBuffer[winIdx], 
                      (byte[])recvBuffer[winIdx], size);
              }
            }

            if (validateData) {
              if(useBufferAPI) {
                fillData((ByteBuffer)sendBuffer[0], (ByteBuffer)recvBuffer[0], 4);
              } else {
                fillData((byte[])sendBuffer[0], (byte[])recvBuffer[0], 4);
              }
            }

            if(useBufferAPI) {
              MPI.COMM_WORLD.recv((ByteBuffer)recvBuffer[0], 4, MPI.BYTE, 1, 101);
            } else {
              MPI.COMM_WORLD.recv((byte[])recvBuffer[0], 4, MPI.BYTE, 1, 101);
            } 

            if (validateData) {
              if(useBufferAPI) {
                validateDataAfterRecv((ByteBuffer)sendBuffer[0], 
                    (ByteBuffer)recvBuffer[0], 4);
              } else {
                validateDataAfterRecv((byte[])sendBuffer[0], 
                    (byte[])recvBuffer[0], 4);
              }
            }

            if(i == (benchWarmupIters + benchIters -1)) {
              totalTime += (System.nanoTime() - init) / (1E9*1.0);
            }

	  } else if(me == 1) {

            if (validateData) {
              if(useBufferAPI) {
                for (int winIdx = 0 ; winIdx < windowSize ; winIdx++) 
                  fillData((ByteBuffer)sendBuffer[winIdx], 
                      (ByteBuffer)recvBuffer[winIdx], size);
              } else {
                for (int winIdx = 0 ; winIdx < windowSize ; winIdx++)
                  fillData((byte[])sendBuffer[winIdx], 
                      (byte[])recvBuffer[winIdx], size);
              }
            }

            if(useBufferAPI) {
              for (int winIdx = 0 ; winIdx < windowSize ; winIdx++) {
                reqs[winIdx] = MPI.COMM_WORLD.iRecv(
                    (ByteBuffer)recvBuffer[winIdx], size, MPI.BYTE, 0, 100);
              }
            } else {
              for (int winIdx = 0 ; winIdx < windowSize ; winIdx++) {
                reqs[winIdx] = MPI.COMM_WORLD.iRecv(
                    (byte[])recvBuffer[winIdx], size, MPI.BYTE, 0, 100);
              }
            }

            Request.waitAllStatus(reqs);

            if (validateData) {
              if(useBufferAPI) {
                for (int winIdx = 0 ; winIdx < windowSize ; winIdx++) {
                  validateDataAfterRecv((ByteBuffer)sendBuffer[winIdx], 
                      (ByteBuffer)recvBuffer[winIdx], size);
                }
              } else { 
                for (int winIdx = 0 ; winIdx < windowSize ; winIdx++)
                  validateDataAfterRecv((byte[])sendBuffer[winIdx], 
                      (byte[])recvBuffer[winIdx], size);
              }
            }

            if (validateData) {
              if(useBufferAPI) {
                fillData((ByteBuffer)sendBuffer[0], (ByteBuffer)recvBuffer[0], 4);
              } else {
                fillData((byte[])sendBuffer[0], (byte[])recvBuffer[0], 4);
              }
            }

            if(useBufferAPI) {
              MPI.COMM_WORLD.send((ByteBuffer)sendBuffer[0], 4, MPI.BYTE, 0, 101);
            } else {
              MPI.COMM_WORLD.send((byte[])sendBuffer[0], 4, MPI.BYTE, 0, 101);
            }

            if (validateData) {
              if(useBufferAPI) {
                validateDataAfterSend((ByteBuffer)sendBuffer[0], 
                    (ByteBuffer)recvBuffer[0], 4);
              } else {
                validateDataAfterSend((byte[])sendBuffer[0], (byte[])recvBuffer[0], 4);
              }
            }

	  }

	}

	if(me == 0) {

	  double bandwidthInMBps = 
              (size / 1e6 * benchIters * windowSize) / totalTime;
          double bandwidthInGbps = (bandwidthInMBps * 8) / 1000;

          System.out.println(size + "\t\t\t" +  
              String.format("%.2f", bandwidthInMBps) + "\t\t\t" +
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
          MPI.COMM_WORLD.recv(
              (ByteBuffer)recvBuffer[i%windowSize], 1024, MPI.BYTE, 1, 998);
          MPI.COMM_WORLD.send(
              (ByteBuffer)sendBuffer[i%windowSize], 1024, MPI.BYTE, 1, 998);
        } else {     
          MPI.COMM_WORLD.recv(
              (byte[])recvBuffer[i%windowSize], 1024, MPI.BYTE, 1, 998);
          MPI.COMM_WORLD.send(
              (byte[])sendBuffer[i%windowSize], 1024, MPI.BYTE, 1, 998);
        }

      } else if(me == 1) {

        if(useBufferAPI) {
          MPI.COMM_WORLD.send(
              (ByteBuffer)sendBuffer[i%windowSize], 1024, MPI.BYTE, 0, 998);
          MPI.COMM_WORLD.recv(
              (ByteBuffer)recvBuffer[i%windowSize], 1024, MPI.BYTE, 0, 998);
        } else { 
          MPI.COMM_WORLD.send(
              (byte[])sendBuffer[i%windowSize], 1024, MPI.BYTE, 0, 998);
          MPI.COMM_WORLD.recv(
              (byte[])recvBuffer[i%windowSize], 1024, MPI.BYTE, 0, 998);
        }

      }

    } //end benchWarmupIters loop
  } 

  protected void printHeader() {
    System.out.println(OSU_BW_TEST);
    System.out.println("# Size [B]" + "\t\t" + "Bandwidth [MB/s]" + "\t" +
        "Bandwidth [Gb/s]");
  }
  
  public static void main(String[] args) throws Exception {
    OSUBandwidth bwTest = new OSUBandwidth(args);

    MPI.Init(args); 

    if (MPI.COMM_WORLD.getRank() == 0)
      bwTest.printHeader();

    if (bwTest.printVersion) {
      if (MPI.COMM_WORLD.getRank() == 0)
        bwTest.printVersion();

      MPI.Finalize();
      return;
    }

    if (bwTest.printHelp) {
      if (MPI.COMM_WORLD.getRank() == 0)
        bwTest.printHelp();

      MPI.Finalize();
      return;
    }

    bwTest.runBenchmark();

    MPI.Finalize();

  }
  
}

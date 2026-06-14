/**
* Copyright (c) 2002-2022 the Network-Based Computing Laboratory
* (NBCL), The Ohio State University.
* 
* Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
* 
* For detailed copyright and licensing information, please refer to the
* copyright file COPYRIGHT in the top level OMB directory.
*/

package mpi.common;

import java.nio.ByteBuffer;
import java.util.StringTokenizer;

public abstract class BenchmarkUtils {

  private final String versionNumber = "6.0";

  public static final String OSU_LAT_TEST = "# OSU Latency Test";
  public static final String OSU_BW_TEST = "# OSU Bandwidth Test";
  public static final String OSU_BIBW_TEST = "# OSU Bi-Bandwidth Test";
  public static final String OSU_OMPI_BW_TEST = "# OSU Open MPI Bandwidth Test";
  public static final String OSU_OMPI_BIBW_TEST = "# OSU Open MPI Bi-Bandwidth Test";

  public static final String OSU_BARRIER_TEST = "# OSU Barrier Test";
  public static final String OSU_BCAST_TEST= "# OSU Bcast Test";
  public static final String OSU_REDUCE_TEST = "# OSU Reduce Test";
  public static final String OSU_ALLREDUCE_TEST = "# OSU Allreduce Test";
  public static final String OSU_GATHER_TEST = "# OSU Gather Test";
  public static final String OSU_GATHERV_TEST = "# OSU Gatherv Test";
  public static final String OSU_ALLGATHER_TEST = "# OSU Allgather Test";
  public static final String OSU_ALLGATHERV_TEST = "# OSU Allgatherv Test";
  public static final String OSU_SCATTER_TEST = "# OSU Scatter Test";
  public static final String OSU_SCATTERV_TEST = "# OSU Scatterv Test";
  public static final String OSU_ALLTOALL_TEST = "# OSU Alltoall Test";
  public static final String OSU_ALLTOALLV_TEST = "# OSU Alltoallv Test";
  public static final String OSU_REDUCESCATTER_TEST = "# OSU ReduceScatter Test";

  protected final int MAX_MESSAGE_SIZE = (1 << 22);
  protected final int MAX_MSG_SIZE_PT2PT = (1<<20);
  protected final int MAX_MSG_SIZE_COLL = (1<<20);
  protected final int MIN_MESSAGE_SIZE = 1;
  protected final int LARGE_MESSAGE_SIZE = 8192;
  protected final int MESSAGE_SIZE_ONE_MB = 1*1024*1024;

  protected final int INITIAL_WARMUP = 10000;
  protected final int INITIAL_WINDOW_SIZE = 64;

  protected final int BW_LOOP_SMALL = 10000;
  protected final int BW_SKIP_SMALL = 1000;
  protected final int BW_LOOP_LARGE = 500;
  protected final int BW_SKIP_LARGE = 100;
  protected final int BW_LOOP_LARGE_ONE_MB = 100;
  protected final int BW_SKIP_LARGE_ONE_MB = 50;

  protected final int LAT_LOOP_SMALL = 10000;
  protected final int LAT_SKIP_SMALL = 1000;
  protected final int LAT_LOOP_LARGE = 500;
  protected final int LAT_SKIP_LARGE = 100;

  protected final int COLL_LOOP_SMALL = 10000;
  protected final int COLL_SKIP_SMALL = 1000;
  protected final int COLL_LOOP_LARGE = 500;
  protected final int COLL_SKIP_LARGE = 100;

  protected int benchIters = 100*1000;
  protected int benchWarmupIters = 10000;
  protected int maxMsgSize = 1*1024*1024;
  protected int minMsgSize = MIN_MESSAGE_SIZE;

  protected final int FLOAT_SIZE = 4;
  protected final int INT_SIZE = 4;

  protected String testName;

  protected boolean printHelp = false;
  protected boolean printVersion = false;
  protected boolean validateData = false;
  protected boolean useBufferAPI = true; 
  protected int windowSize = INITIAL_WINDOW_SIZE;
  
  protected BenchmarkUtils() {

  }

  protected BenchmarkUtils(String[] args, String tName) {

    this.testName = tName;
 
    switch(testName) {
      case OSU_LAT_TEST: 
        benchIters = LAT_LOOP_SMALL;
        benchWarmupIters = LAT_SKIP_SMALL;
        maxMsgSize = MAX_MESSAGE_SIZE;
        break; 

      case OSU_BW_TEST:
      case OSU_BIBW_TEST:
      case OSU_OMPI_BW_TEST:
      case OSU_OMPI_BIBW_TEST:
        benchIters = BW_LOOP_SMALL;
        benchWarmupIters = BW_SKIP_SMALL;
        maxMsgSize = MAX_MESSAGE_SIZE;
        break; 

      case OSU_REDUCESCATTER_TEST:
      case OSU_ALLTOALL_TEST: 
      case OSU_ALLTOALLV_TEST: 
      case OSU_SCATTER_TEST: 
      case OSU_SCATTERV_TEST: 
      case OSU_ALLGATHER_TEST: 
      case OSU_ALLGATHERV_TEST: 
      case OSU_GATHER_TEST: 
      case OSU_GATHERV_TEST: 
      case OSU_ALLREDUCE_TEST: 
      case OSU_REDUCE_TEST: 
      case OSU_BCAST_TEST: 
      case OSU_BARRIER_TEST: 
        benchIters = COLL_LOOP_SMALL;
        benchWarmupIters = COLL_SKIP_SMALL;
        maxMsgSize = MAX_MSG_SIZE_COLL;
        break; 

      default:
        System.out.println("The test <"+testName+"> is not supported by Java OMB");
        System.exit(0);
    }

    for (int i = 0; i < args.length; i++) {

      if (args[i].equals("-x") || args[i].equals("--warmup")) {

        benchWarmupIters = Integer.parseInt(args[i+1]);
        i++;

      } else if (args[i].equals("-i") || args[i].equals("--iterations")) {

        benchIters = Integer.parseInt(args[i+1]);
        i++;

      } else if (args[i].equals("-u") || args[i].equals("--validation-warmup")) { // stats

        System.out.println(args[i] + " option is not supported");
        i++;

      } else if (args[i].equals("-m") || args[i].equals("--message-size")) { // msg sizes
        String messageSizeInput = args[i+1];
        
        if(!messageSizeInput.contains(":")) {
	  maxMsgSize = Integer.parseInt(messageSizeInput);
        } else if(messageSizeInput.endsWith(":")) {
          messageSizeInput = messageSizeInput.replaceAll(":", "");
	  minMsgSize = Integer.parseInt(messageSizeInput);
        } else {
          StringTokenizer st = new StringTokenizer(messageSizeInput, ":");
          minMsgSize = Integer.parseInt(st.nextToken());
          maxMsgSize = Integer.parseInt(st.nextToken());
        }
        i++;
      } else if (args[i].equals("-M") || args[i].equals("--mem-limit")) { // memory
        System.out.println("-M option is not supported");
        i++;
      } else if (args[i].equals("-h") || args[i].equals("--help")) { // help
        printHelp = true;
      } else if (args[i].equals("-W") || args[i].equals("--window-size")) {
        windowSize = Integer.parseInt(args[i+1]);
        i++;
        if(!testName.equals(OSU_BW_TEST))
	  System.out.println(args[i] + " is not needed for this test");
      } else if (args[i].equals("-c") || args[i].equals("--validation")) {
        validateData = true;
      } else if (args[i].equals("-a")) {
        String apiToUse = args[i+1];

        if (apiToUse.equals("buffer"))
          useBufferAPI = true;
        else if (apiToUse.equals("arrays"))
          useBufferAPI = false;
        else {
	  System.out.println("Incorrect option specified <"+apiToUse+">");
	  System.out.println("Continuing with the buffer API");
          useBufferAPI = true;
        }

        i++;
      } else if (args[i].equals("-v") || args[i].equals("--version")) { // version
        printVersion = true;
      } else if (args[i].equals("-b")) { // multi buffer
        System.out.println("-b option is not supported");
        i++;
      }
    }
  }

  protected void printVersion() {
    System.out.println("# "+testName+" "+versionNumber);
  }

  protected abstract void printHeader();

  protected void printHelp() {

    System.out.print("  -m, --message-size          [MIN:]MAX  set the minimum and/or the \n"+
                     "                              maximum message size to MIN and/or MAX\n"+
                     "                              bytes respectively. Examples:\n"+
                     "                              -m 128      // min = default, max = 128\n"+
                     "                              -m 2:128    // min = 2, max = 128\n"+
                     "                              -m 2:       // min = 2, max = default\n");
    System.out.print("  -x, --warmup ITER                     number of warmup iterations to skip before\n"+ 
                     "                              timing (default 10000)\n");
    System.out.print("  -i, --iterations ITER       number of iterations for timing\n"+
                     "                              (default 10000)\n");

    if (testName.equals(OSU_BW_TEST) || 
       testName.equals(OSU_BIBW_TEST) ||
       testName.equals(OSU_OMPI_BW_TEST) ||
       testName.equals(OSU_OMPI_BIBW_TEST)) {

      System.out.print("  -W, --window-size SIZE      set number of messages to send before\n"+
                       "                              synchronization (default 64)");

    } 

    System.out.print("  -a API                      the api to use for exchanging data. Options\n"+
                     "                              are 'buffer' or 'arrays' APIs. (default buffer)\n");      
    System.out.print("  -c, --validation            validates exchanged data \n");
    System.out.print("  -h, --help                  print this help message\n");
    System.out.print("  -v, --version               print the version info\n");
  }

  protected void fillData(ByteBuffer sBuffer, ByteBuffer rBuffer, int count) {
    for(int i=0 ; i<count ; i++) {
      sBuffer.put((byte)1);
      rBuffer.put((byte)0);
    }
  }

  protected void fillData(ByteBuffer buffer, byte b, int count) {
    for(int i=0 ; i<count ; i++) {
      buffer.put(b);
    }
  }

  protected void fillData(byte[] sBuffer, byte[] rBuffer, int count) {
    for(int i=0 ; i<count ; i++) {
      sBuffer[i] = (byte)1;
      rBuffer[i] = (byte)0;
    }
  }

  protected void fillData(byte[] buffer, byte b, int count) {
    for(int i=0 ; i<count ; i++) {
      buffer[i] = b;
    }
  }

  protected void fillSendAndRecvBufferForReduce(ByteBuffer sendBuffer,  
      ByteBuffer recvBuffer, int count) {
    for(int i=0 ; i<count ; i++) {
      float src = i*1.0f;
      float dst = i*0.0f;
      sendBuffer.putFloat(src);
      recvBuffer.putFloat(dst);
    } 
  }

  protected void fillSendAndRecvBufferForReduceScatter(ByteBuffer sendBuffer,  
      ByteBuffer recvBuffer, int count, int rCount) {

    for(int i=0 ; i<count ; i++) {
      float src = i*1.0f;
      sendBuffer.putFloat(src);
    } 

    for(int i=0 ; i < rCount ; i++) {
      float dst = i*0.0f;
      recvBuffer.putFloat(dst);
    }

  }

  protected void fillSendAndRecvBufferForReduce(float[] sendBuffer,  
      float[] recvBuffer, int count) {
    for(int i=0 ; i<count ; i++) {
      float src = i*1.0f;
      float dst = i*0.0f;
      sendBuffer[i] = src;
      recvBuffer[i] = dst;
    } 
  }

  protected void fillSendAndRecvBufferForReduceScatter(float[] sendBuffer,  
      float[] recvBuffer, int count, int rCount) {
    for(int i=0 ; i<count ; i++) {
      float src = i*1.0f;
      sendBuffer[i] = src;
    } 

    for(int i=0 ; i < rCount ; i++) {
      float dst = i*0.0f;
      recvBuffer[i] = dst;
    }
  }

  protected void validateDataAfterReduce(ByteBuffer sendBuffer,
      ByteBuffer recvBuffer, int count, int totalProcs) {
    
    for(int i=0 ; i<count; i++) {

      float src = sendBuffer.getFloat(); 
      float dst = recvBuffer.getFloat(); 

      if((src*totalProcs*1.0f) != dst) {
        System.out.println("data validation failed: idx <"+i+
                           ">, src=<"+(src*totalProcs*1.0f)+">, dst=<"+dst+">");
      }
    }

  }

  protected void validateDataAfterReduce(float[] sendBuffer,
      float[] recvBuffer, int count, int totalProcs) {

    for(int i=0 ; i<count ; i++) {

      float src = sendBuffer[i];
      float dst = recvBuffer[i];

      if((src*totalProcs*1.0f) != dst) {
        System.out.println("data validation failed: idx <"+i+
                           ">, src=<"+(src*totalProcs*1.0f)+">, dst=<"+dst+">");
      }
    }

  }


  protected boolean validateDataAfterSend(ByteBuffer src, ByteBuffer dst, 
      int count) {

    byte srcByte, dstByte;

    src.flip();
    dst.flip();
    
    for(int i=0 ; i<count ; i++) {
      srcByte = src.get();
      dstByte = dst.get();
      if(srcByte == dstByte) {
        System.out.println("data validation failed: idx <"+i+">, srcByte="+srcByte+
                           ", dstByte="+dstByte+">, src=<"+src+">, dst=<"+dst+">");
        return false;
      }
    }

    src.clear();
    dst.clear();

    return true;
  }

  protected boolean validateDataAfterSend(ByteBuffer src, int count) {

    byte srcByte;

    src.flip();
    
    for(int i=0 ; i<count ; i++) {
      srcByte = src.get();
      if(srcByte != ((byte)1)) {
        System.out.println("data validation failed: idx <"+i+">, srcByte="+srcByte+
                           ", should be 1, src=<"+src+">");
        return false;
      }
    }

    src.clear();

    return true;
  }

  protected boolean validateDataAfterSend(byte[] src, byte[] dst, int count) {

    byte srcByte, dstByte;

    for(int i=0 ; i<count ; i++) {
      srcByte = src[i];
      dstByte = dst[i];
      if(srcByte == dstByte) {
        System.out.println("data validation failed: idx <"+i+">, srcByte="+srcByte+
                           ", dstByte="+dstByte+">, src=<"+src+">, dst=<"+dst+">");
        return false;
      }
    }

    return true;
  }

  protected boolean validateDataAfterSend(byte[] src, int count) {

    byte srcByte;

    for(int i=0 ; i<count ; i++) {
      srcByte = src[i];
      if(srcByte != ((byte)1)) {
        System.out.println("data validation failed: idx <"+i+">, srcByte="+srcByte+
                           ", should be 1, src=<"+src+">");
        return false;
      }
    }

    return true;
  }

  protected boolean validateDataAfterRecv(ByteBuffer src, ByteBuffer dst, 
      int count) {

    byte srcByte, dstByte;

    src.flip();
    dst.flip();

    for(int i=0 ; i<count ; i++) {
      srcByte = src.get();
      dstByte = dst.get();
      if(srcByte != dstByte) {
        System.out.println("data validation failed: idx <"+i+">, srcByte="+srcByte+
                           ", dstByte="+dstByte+">, src=<"+src+">, dst=<"+dst+">");
        return false;
      }
    }

    src.clear();
    dst.clear(); 
   
    return true;
  }

  protected boolean validateDataAfterRecv(ByteBuffer dst, int count) {

    byte dstByte;

    dst.flip();

    for(int i=0 ; i<count ; i++) {
      dstByte = dst.get();
      if(dstByte != ((byte)1)) {
        System.out.println("data validation failed: idx <"+i+
            ">, dstByte="+dstByte+">, should be 1, dst=<"+dst+">");
        return false;
      }
    }

    dst.clear(); 
   
    return true;
  }

  protected boolean validateDataAfterRecv(byte[] src, byte[] dst, int count) {
    byte srcByte, dstByte;
    for(int i=0 ; i<count ; i++) {
      srcByte = src[i];
      dstByte = dst[i];
      if(srcByte != dstByte) {
        System.out.println("data validation failed: idx <"+i+">, srcByte="+srcByte+
                           ", dstByte="+dstByte+">, src=<"+src+">, dst=<"+dst+">");
        return false;
      }
    }
    return true;
  }

  protected boolean validateDataAfterRecv(byte[] dst, int count) {
    byte dstByte;
    for(int i=0 ; i<count ; i++) {
      dstByte = dst[i];
      if(dstByte != ((byte)1)) {
        System.out.println("data validation failed: idx <"+i+">, dstByte="+
            dstByte+">, should be 1, dst=<"+dst+">");
        return false;
      }
    }
    return true;
  }

}

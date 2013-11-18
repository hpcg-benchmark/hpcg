
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "ReadHpcgDat.hpp"

const char *HPCG_DAT = "hpcg.dat";

static int
WriteHpcgDat(const char *s, size_t length) {
  FILE *stream = fopen(HPCG_DAT, "wb");

  if (! stream) {
    fprintf(stderr, "Cannot write %s\n", HPCG_DAT);
    exit(127);
    return -1;
  }

  fwrite(s, 1, length, stream);

  fclose(stream);

  return 0;
}

static void
RemoveHpcgDat(void) {
  remove(HPCG_DAT);
}

// Test for empty hpcg.dat
int
TestCase1(void) {
  const char *s = "";
  WriteHpcgDat(s, strlen(s));
  int localDims[3] = {-23, -29, -31}, seconds = -7;

  ReadHpcgDat(localDims, &seconds);
  printf( "%d %d %d %d\n", localDims[0], localDims[1], localDims[2], seconds ); fflush(stdout);

  int retVal = 0, idx = 0;
  if (localDims[0] < 1) retVal |= 1 << (idx++);
  if (localDims[1] < 1) retVal |= 1 << (idx++);
  if (localDims[2] < 1) retVal |= 1 << (idx++);
  if (seconds < 1) retVal |= 1 << (idx++);
  RemoveHpcgDat();

  return retVal;
}

// Test for simple hpcg.dat with Unix line endings: \n
int
TestCase2(void) {
  const char *s = "First\nSecond\n13 17 19\n3613\n";
  WriteHpcgDat(s, strlen(s));

  int localDims[3] = {23, 29, 31}, seconds = 7;
  int retVal = 0, idx = 0;

  ReadHpcgDat(localDims, &seconds);
  printf( "%d %d %d %d\n", localDims[0], localDims[1], localDims[2], seconds ); fflush(stdout);

  if (localDims[0] != 13) retVal |= 1 << (idx++);

  if (localDims[1] != 17) retVal |= 1 << (idx++);

  if (localDims[2] != 19) retVal |= 1 << (idx++);

  if (seconds != 3613) retVal |= 1 << (idx++);

  RemoveHpcgDat();

  return retVal;
}

// Test for simple hpcg.dat with Windows line endings: \r\n
int
TestCase3(void) {
  const char *s = "First\r\nSecond\r\n13 17 19\r\n3613\r\n";
  WriteHpcgDat(s, strlen(s));

  int localDims[3] = {23, 29, 31}, seconds = 7;
  int retVal = 0, idx = 0;

  ReadHpcgDat(localDims, &seconds);
  printf( "%d %d %d %d\n", localDims[0], localDims[1], localDims[2], seconds ); fflush(stdout);

  if (localDims[0] != 13) retVal |= 1 << (idx++);

  if (localDims[1] != 17) retVal |= 1 << (idx++);

  if (localDims[2] != 19) retVal |= 1 << (idx++);

  if (seconds != 3613) retVal |= 1 << (idx++);

  RemoveHpcgDat();

  return retVal;
}

// Test for simple hpcg.dat with old Mac OS line endings: \r
int
TestCase4(void) {
  const char *s = "First\rSecond\r13 17 19\r3613\r";
  WriteHpcgDat(s, strlen(s));

  int localDims[3] = {23, 29, 31}, seconds = 7;
  int retVal = 0, idx = 0;

  ReadHpcgDat(localDims, &seconds);
  printf( "%d %d %d %d\n", localDims[0], localDims[1], localDims[2], seconds ); fflush(stdout);

  if (localDims[0] != 13) retVal |= 1 << (idx++);

  if (localDims[1] != 17) retVal |= 1 << (idx++);

  if (localDims[2] != 19) retVal |= 1 << (idx++);

  if (seconds != 3613) retVal |= 1 << (idx++);

  RemoveHpcgDat();

  return retVal;
}

int main(void) {
  int mainReturnValue = 0;

  int (*testCases[])(void) = {
    TestCase1,
    TestCase2,
    TestCase3,
    TestCase4,
    0
  };

  for (int i=0; testCases[i]; ++i) {
    int retVal = testCases[i]();
    if (retVal) {
      fprintf(stderr, "Test case %d returned %d\n", i+1, retVal);
      mainReturnValue = 129;
    } else
      fprintf(stderr, "Test case %d succeeded\n", i+1);
    fflush(stderr);
  }

  return mainReturnValue;
}

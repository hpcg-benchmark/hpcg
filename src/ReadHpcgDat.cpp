
//@HEADER
// ***************************************************
//
// HPCG: High Performance Conjugate Gradient Benchmark
//
// Contact:
// Michael A. Heroux ( maherou@sandia.gov)
// Jack Dongarra     (dongarra@eecs.utk.edu)
// Piotr Luszczek    (luszczek@eecs.utk.edu)
//
// ***************************************************
//@HEADER

#include <cstdio>

#include "ReadHpcgDat.hpp"

static int
SkipUntilEol(FILE *stream) {
  int chOrEof;
  bool finished;

  do {
    chOrEof = fgetc( stream );
    finished = (chOrEof == EOF) || (chOrEof == '\n') || (chOrEof == '\r');
  } while (! finished);

  if ('\r' == chOrEof) { // on Windows, \r might be followed by \n
    int chOrEofExtra = fgetc( stream );

    if ('\n' == chOrEofExtra || EOF == chOrEofExtra)
      chOrEof = chOrEofExtra;
    else
      ungetc(chOrEofExtra, stream);
  }

  return chOrEof;
}

int
ReadHpcgDat(int *localDimensions, int *secondsPerRun, int *localProcDimensions) {
  FILE * hpcgStream = fopen("hpcg.dat", "r");

  if (! hpcgStream)
    return -1;

  SkipUntilEol(hpcgStream); // skip the first line

  SkipUntilEol(hpcgStream); // skip the second line

  for (int i = 0; i < 3; ++i)
    if (fscanf(hpcgStream, "%d", localDimensions+i) != 1 || localDimensions[i] < 16)
      localDimensions[i] = 16;

  SkipUntilEol( hpcgStream ); // skip the rest of the second line

  if (secondsPerRun!=0) { // Only read number of seconds if the pointer is non-zero
    if (fscanf(hpcgStream, "%d", secondsPerRun) != 1 || secondsPerRun[0] < 0)
      secondsPerRun[0] = 30 * 60; // 30 minutes
  }

  SkipUntilEol( hpcgStream ); // skip the rest of the third line

  for (int i = 0; i < 3; ++i)
    // the user didn't specify (or values are invalid) process dimensions
    if (fscanf(hpcgStream, "%d", localProcDimensions+i) != 1 || localProcDimensions[i] < 1)
      localProcDimensions[i] = 0; // value 0 means: "not specified" and it will be fixed later

  fclose(hpcgStream);

  return 0;
}

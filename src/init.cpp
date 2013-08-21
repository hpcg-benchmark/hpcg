
#include <ctime>
#include <cstring>

#include <fstream>

#ifndef HPCG_NOMPI
#include <mpi.h>
#endif

#include "hpcg.hpp"

std::ofstream HPCG_fout;

int
HPCG_Init(void) {
  char fname[80], local[15];
  time_t rawtime;
  tm *ptm;

#ifdef HPCG_NOMPI
  local[0] = 0;
#else
  int rank;

  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  sprintf( local, "%d_", rank );
#endif

  time ( &rawtime );
  ptm = localtime(&rawtime);
  sprintf( fname, "hpcg_log_%s%04d%02d%02d_%02d:%02d:%02d.txt", local,
      1900 + ptm->tm_year, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec );

  HPCG_fout.open(fname);

  return 0;
}

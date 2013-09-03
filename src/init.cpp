
#include <ctime>
#include <cstdio>
#include <cstring>

#include <fstream>

#ifndef HPCG_NOMPI
#include <mpi.h>
#endif

#include "hpcg.hpp"

std::ofstream HPCG_fout;

static int
startswith(const char *s, const char *prefix) {
    size_t n = strlen( prefix );
    if (strncmp( s, prefix, n ))
        return 0;
    return 1;
}

int
HPCG_Init(int *argc_p, char ***argv_p, HPCG_Params *params) {
  int argc = *argc_p;
  char **argv = *argv_p;
  char fname[80], local[15];
  int i, j, iparams[3];
  char cparams[3][6] = {"--nx=", "--ny=", "--nz="};
  time_t rawtime;
  tm *ptm;

  /* for sequential and some MPI implementations it's OK to read first three args */
  for (i = 0; i < 3; ++i)
    if (argc <= i+1 || sscanf(argv[i+1], "%d", iparams+i) != 1 || iparams[i] < 10) iparams[i] = 0;

  /* for some MPI environments, command line arguments may get complicated so we need a prefix */
  for (i = 1; i <= argc && argv[i]; ++i)
    for (j = 0; j < 3; ++j)
      if (startswith(argv[i], cparams[j]))
        if (sscanf(argv[i]+strlen(cparams[j]), "%d", iparams+j) != 1 || iparams[j] < 10) iparams[j] = 0;

  if (! iparams[0] && ! iparams[1] && ! iparams[2]) { /* no geometry arguments on the command line */
    FILE *f = fopen("hpcg.dat", "r");
    if (f) {
      for (i = 0; i < 3; ++i)
        if (fscanf(f, "%d", iparams+i) != 1 || iparams[i] < 10) iparams[i] = 0;

      fclose(f);
    }
  }

  for (i = 0; i < 3; ++i) {
    if (iparams[i] < 10)
      for (j = 1; j <= 2; ++j)
        if (iparams[(i+j)%3] > iparams[i])
          iparams[i] = iparams[(i+j)%3];
    if (iparams[i] < 10)
      iparams[i] = 10;
  }

#ifndef HPCG_NOMPI
  MPI_Bcast( iparams, 3, MPI_INT, 0, MPI_COMM_WORLD );
#endif

  params->nx = iparams[0];
  params->ny = iparams[1];
  params->nz = iparams[2];

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

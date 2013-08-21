
#include <fstream>

#include "hpcg.hpp"

int
HPCG_Finalize(void) {
  HPCG_fout.close();
}

/*!
 @file finalize.cpp

 HPCG routine
 */

#include <fstream>

#include "hpcg.hpp"

/*!
  Closes the I/O stream used for logging information throughout the HPCG run.

  @see HPCG_Init
*/
int
HPCG_Finalize(void) {
  HPCG_fout.close();
  return(0);
}

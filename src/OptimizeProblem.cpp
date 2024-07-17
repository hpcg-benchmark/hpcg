
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

/*!
 @file OptimizeProblem.cpp

 HPCG routine
 */

#include "OptimizeProblem.hpp"
#include <iostream>
/*!
  Optimizes the data structures used for CG iteration to increase the
  performance of the benchmark version of the preconditioned CG algorithm.

  @param[inout] A      The known system matrix, also contains the MG hierarchy in attributes Ac and mgData.
  @param[inout] data   The data structure with all necessary CG vectors preallocated
  @param[inout] b      The known right hand side vector
  @param[inout] x      The solution vector to be computed in future CG iteration
  @param[inout] xexact The exact solution vector

  @return returns 0 upon success and non-zero otherwise

  @see GenerateGeometry
  @see GenerateProblem
*/
int OptimizeProblem(SparseMatrix & A, CGData & data, Vector & b, Vector & x, Vector & xexact) {

  // This function can be used to completely transform any part of the data structures.
  // Right now it does nothing, so compiling with a check for unused variables results in complaints

#ifdef HPCG_USE_MULTICOLORING
  const local_int_t nrow = A.localNumberOfRows;
  std::vector<local_int_t> colors(nrow, nrow); // value `nrow' means `uninitialized'; initialized colors go from 0 to nrow-1
  int totalColors = 1;
  colors[0] = 0; // first point gets color 0

  // Finds colors in a greedy (a likely non-optimal) fashion.

  for (local_int_t i=1; i < nrow; ++i) {
    if (colors[i] == nrow) { // if color not assigned
      std::vector<int> assigned(totalColors, 0);
      int currentlyAssigned = 0;
      const local_int_t * const currentColIndices = A.mtxIndL[i];
      const int currentNumberOfNonzeros = A.nonzerosInRow[i];

      for (int j=0; j< currentNumberOfNonzeros; j++) { // scan neighbors
        local_int_t curCol = currentColIndices[j];
        if (curCol < i) { // if this point has an assigned color (points beyond `i' are unassigned)
          if (assigned[colors[curCol]] == 0)
            currentlyAssigned += 1;
          assigned[colors[curCol]] = 1; // this color has been used before by `curCol' point
        } // else // could take advantage of indices being sorted
      }

      if (currentlyAssigned < totalColors) { // if there is at least one color left to use
        for (int j=0; j < totalColors; ++j)  // try all current colors
          if (assigned[j] == 0) { // if no neighbor with this color
            colors[i] = j;
            break;
          }
      } else {
        if (colors[i] == nrow) {
          colors[i] = totalColors;
          totalColors += 1;
        }
      }
    }
  }


  /* colors[i] save the color of row "i" */
  /* counters[i] save the number of rows assigned to color "i" */
  std::vector<local_int_t> counters(totalColors);
  for (local_int_t i=0; i<nrow; ++i)
    counters[colors[i]]++;

  /* colorIndices save the first row with color "i" */
  /* computationOrder save the order that used in SYMGS */
  std::vector<local_int_t> colorIndices(totalColors, 0);
  local_int_t old = 0, old0;
  for ( int i = 1; i < totalColors; i++ ) {
  	old0 = counters[i];
  	counters[i] = counters[i-1] + old;
  	colorIndices[i] = counters[i];
  	old = old0;
  }
  counters[0] = 0;
  colorIndices[0] = 0;

  std::vector<local_int_t> computationOrder(nrow);
  local_int_t j = 0;
  for ( int ic = 0; ic < colorIndices.size(); ic++ ) {
  	for ( local_int_t i = 0; i < nrow; i++ ) {
  		if ( colors[i] == ic ) {
  			computationOrder[j] = i;
  			j++;
  		}
  	}
  }

  /* save the metadata to Matrix */
  A.optimizationData[0] = std::vector<local_int_t>(computationOrder);
  A.optimizationData[1] = std::vector<local_int_t>(colorIndices);

#elif HPCG_USE_REORDER_MULTICOLORING

	const local_int_t nrow = A.localNumberOfRows;

	// On the finest grid we use TDG algorithm
	A.TDG = true;

	// Create an auxiliary vector to store the number of dependencies on L for every row
	std::vector<unsigned char> nonzerosInLowerDiagonal(nrow, 0);

	/*
	 * Now populate these vectors. This loop is safe to parallelize
	 */
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
	for ( local_int_t i = 0; i < nrow; i++ ) {
		for ( local_int_t j = 0; j < A.nonzerosInRow[i]; j++ ) {
			local_int_t curCol = A.mtxIndL[i][j];
			if ( curCol < i && curCol < nrow ) { // check that it's on L and not a row from other domain
				nonzerosInLowerDiagonal[i]++;
			} else if ( curCol == i ) { // we found the diagonal, no more L dependencies from here
				break;
			}
		}
	}

	std::vector<local_int_t> depsVisited(nrow, 0);
	std::vector<bool> processed(nrow, false);
	local_int_t rowsProcessed = 0;
	
	// Allocate the TDG structure. Starts as an empty matrix
	A.tdg = std::vector<std::vector<local_int_t> >();

	// We start by adding the first row of the grid to the first level. This row has no L dependencies
	std::vector<local_int_t> aux(1, 0);
	A.tdg.push_back(aux);
	// Increment the number of dependencies visited for each of the neighbours
	for ( local_int_t j = 0; j < A.nonzerosInRow[0]; j++ ) {
		if ( A.mtxIndL[0][j] != 0 && A.mtxIndL[0][j] < nrow ) depsVisited[A.mtxIndL[0][j]]++; // don't update deps from other domains
	}
	processed[0] = true;
	rowsProcessed++;

	// Continue with the creation of the TDG
	while ( rowsProcessed < nrow ) {
		std::vector<local_int_t> rowsInLevel;

		// Check for the dependencies of the rows of the level before the current one. The dependencies
		// of these rows are the ones that could have their dependencies fulfilled and therefore added to the
		// current level
		unsigned int lastLevelOfTDG = A.tdg.size()-1;
		for ( local_int_t i = 0; i < A.tdg[lastLevelOfTDG].size(); i++ ) {
			local_int_t row = A.tdg[lastLevelOfTDG][i];

			for ( local_int_t j = 0; j < A.nonzerosInRow[row]; j++ ) {
				local_int_t curCol = A.mtxIndL[row][j];

				if ( curCol < nrow ) { // don't process external domain rows
					// If this neighbour hasn't been processed yet and all its L dependencies has been processed
					if ( !processed[curCol] && depsVisited[curCol] == nonzerosInLowerDiagonal[curCol] ) {
						rowsInLevel.push_back(curCol); // add the row to the new level
						processed[curCol] = true; // mark the row as processed
					}
				}
			}
		}

		// Update some information
		for ( local_int_t i = 0; i < rowsInLevel.size(); i++ ) {
			rowsProcessed++;
			local_int_t row = rowsInLevel[i];
			for ( local_int_t j = 0; j < A.nonzerosInRow[row]; j++ ) {
				local_int_t curCol = A.mtxIndL[row][j];
				if ( curCol < nrow && curCol != row ) {
					depsVisited[curCol]++;
				}
			}
		}

		// Add the just created level to the TDG structure
		A.tdg.push_back(rowsInLevel);
	}

	// Now we need to create some structures to translate from old and new order (yes, we will reorder the matrix)
	A.whichNewRowIsOldRow = std::vector<local_int_t>(A.localNumberOfColumns);
	A.whichOldRowIsNewRow = std::vector<local_int_t>(A.localNumberOfColumns);

	local_int_t oldRow = 0;
	for ( local_int_t level = 0; level < A.tdg.size(); level++ ) {
		for ( local_int_t i = 0; i < A.tdg[level].size(); i++ ) {
			local_int_t newRow = A.tdg[level][i];
			A.whichOldRowIsNewRow[oldRow] = newRow;
			A.whichNewRowIsOldRow[newRow] = oldRow++;
		}
	}

	// External domain rows are not reordered, thus they keep the same ID
	for ( local_int_t i = nrow; i < A.localNumberOfColumns; i++ ) {
		A.whichOldRowIsNewRow[i] = i;
		A.whichNewRowIsOldRow[i] = i;
	}

	// Now we need to allocate some structure to temporary allocate the reordered structures
	double **matrixValues = new double*[nrow];
	local_int_t **mtxIndL = new local_int_t*[nrow];
	char *nonzerosInRow = new char[nrow];
	for ( local_int_t i = 0; i < nrow; i++ ) {
		matrixValues[i] = new double[27];
		mtxIndL[i] = new local_int_t[27];
	}

	// And finally we reorder (and translate at the same time)
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
	for ( local_int_t level = 0; level < A.tdg.size(); level++ ) {
		for ( local_int_t i = 0; i < A.tdg[level].size(); i++ ) {
			local_int_t oldRow = A.tdg[level][i];
			local_int_t newRow = A.whichNewRowIsOldRow[oldRow];

			nonzerosInRow[newRow] = A.nonzerosInRow[oldRow];
			for ( local_int_t j = 0; j < A.nonzerosInRow[oldRow]; j++ ) {
				local_int_t curOldCol = A.mtxIndL[oldRow][j];
				matrixValues[newRow][j] = A.matrixValues[oldRow][j];
				mtxIndL[newRow][j] = curOldCol < nrow ? A.whichNewRowIsOldRow[curOldCol] : curOldCol; // don't translate if row is external
			}
		}
	}

	// time to replace structures
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
	for ( local_int_t i = 0; i < nrow; i++ ) {
		A.nonzerosInRow[i] = nonzerosInRow[i];
		for ( local_int_t j = 0; j < A.nonzerosInRow[i]; j++ ) {
			A.matrixValues[i][j] = matrixValues[i][j];
			A.mtxIndL[i][j] = mtxIndL[i][j];
		}
		// Put some zeros on padding positions
		for ( local_int_t j = A.nonzerosInRow[i]; j < 27; j++ ) {
			A.matrixValues[i][j] = 0.0;
			A.mtxIndL[i][j] = 0;
		}
	}

	// Regenerate the diagonal
	for ( local_int_t i = 0; i < nrow; i++ ) {
		for ( local_int_t j = 0; j < A.nonzerosInRow[i]; j++ ) {
			local_int_t curCol = A.mtxIndL[i][j];
			if ( i == curCol ) {
				A.matrixDiagonal[i] = &A.matrixValues[i][j];
			}
		}
	}

	// Translate TDG row IDs
	oldRow = 0;
	for ( local_int_t l = 0; l < A.tdg.size(); l++ ) {
		for ( local_int_t i = 0; i < A.tdg[l].size(); i++ ) {
			A.tdg[l][i] = oldRow++;
		}
	}

#ifndef HPCG_NO_MPI
	// Translate the row IDs that will be send to other domains
	for ( local_int_t i = 0; i < A.totalToBeSent; i++ ) {
		local_int_t orig = A.elementsToSend[i];
		A.elementsToSend[i] = A.whichNewRowIsOldRow[orig];
	}
#endif

	// Reorder b (RHS) vector
	Vector bReorder;
	InitializeVector(bReorder, b.localLength);
	CopyVector(b, bReorder);
	CopyAndReorderVector(bReorder, b, A.whichNewRowIsOldRow);


#endif

  return 0;
}

// Helper function (see OptimizeProblem.hpp for details)
double OptimizeProblemMemoryUse(const SparseMatrix & A) {

  return 0.0;

}


//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

/*!
 @file TestNorms.cpp

 HPCG routine
 */

#include <cmath>
#include "TestNorms.hpp"

/*!
  Computes the mean and standard deviation of the array of norm results.

  @param[in] testnorms_data data structure with the results of norm test

  @return Returns 0 upon success or non-zero otherwise
*/
int TestNorms(TestNormsData * testnorms_data) {

	// Compute mean
	double mean = 0.0;
	for (int i= 0; i<testnorms_data->samples; ++i) mean += testnorms_data->values[i];
	testnorms_data->mean = (mean/((double)testnorms_data->samples));

	// Compute variance
	double sumdiff = 0.0;
	for (int i= 0; i<testnorms_data->samples; ++i) sumdiff += (testnorms_data->values[i] - mean) * (testnorms_data->values[i] - mean);
	testnorms_data->variance = (sumdiff/((double)testnorms_data->samples));

	// Determine if variation is sufficiently small to declare success
	testnorms_data->pass = (testnorms_data->variance<1.0e-6);

    return 0;
}

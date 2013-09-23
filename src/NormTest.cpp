
//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

/*!
 @file CG.cpp

 HPCG routine
 */

// Changelog
//
// Version 0.1
//
/////////////////////////////////////////////////////////////////////////

#include <cmath>
#include "NormTest.hpp"

/*!
  Computes the mean and standard deviation of the array of norm results.

  @param[in] normtest_data data structure with the norm test results

  @return Returns 0 upon success or non-zero otherwise
*/
int NormTest(NormTestData * normtest_data) {

	// Compute mean
	double mean = 0.0;
	for (int i= 0; i<normtest_data->samples; ++i) mean += normtest_data->values[i];
	normtest_data->mean = (mean/((double)normtest_data->samples));

	// Compute variance
	double sumdiff = 0.0;
	for (int i= 0; i<normtest_data->samples; ++i) sumdiff += (normtest_data->values[i] - mean) * (normtest_data->values[i] - mean);
	normtest_data->variance = (sumdiff/((double)normtest_data->samples));

	// Determine if variation is sufficiently small to declare success
	normtest_data->pass = (normtest_data->variance<1.0e-6);

    return 0;
}

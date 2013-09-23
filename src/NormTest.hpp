
//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

#ifndef NORMTEST_HPP
#define NORMTEST_HPP


struct NormTestData_STRUCT {
  double * values; //!< sample values
  double mean;     //!< mean of all sampes
  double variance; //!< variance of mean
  int samples;     //!< number of samples
  bool pass;       //!< pass/fail indicator
};
typedef struct NormTestData_STRUCT NormTestData;

extern int NormTest(NormTestData * normtest_data);

#endif  // NORMTEST_HPP

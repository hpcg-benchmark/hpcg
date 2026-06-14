########################################################
# HPCG-NVIDIA
NVIDIA accelerated version of:

########################################################
# High Performance Conjugate Gradient Benchmark (HPCG) #
########################################################

Jack Dongarra and Michael Heroux and Piotr Luszczek

Revision: 3.1

Date: March 28, 2019

Updated for HPCG-NVIDIA compatibility

## Introduction ##

HPCG-NVIDIA is a software package that performs a fixed 
number of  multigrid  preconditioned (using a symmetric 
Gauss-Seidel  smoother) conjugate gradient (PCG) itera-
tions using double  precision (64 bit)   floating point
values.

The HPCG rating is a weighted GFLOP/s (billion floating
operations  per second) value that is  composed of the 
operations  performed in the PCG  iteration phase over 
the  time taken.  The overhead time of  problem constr-
uction and any modifications to improve performance are
divided by 500 iterations (the amortization weight) and
added to the runtime.

Integer arrays have global and local scope (global ind-
ices are unique across the entire distributed memory 
system, local indices are unique within a memory image).
Integer data for global/local indices have three modes:

* 32/32 - global and local integers are 32-bit
* 64/32 - global integers are 64-bit, local are 32-bit
* 64/64 - global and local are 64-bit.

These various modes are required in order to address suf-
ficiently big problems if the range of indexing goes 
above 2^31 (roughly 2.1B), or to conserve storage costs
if the range of indexing is less than 2^31.

The HPCG software package requires the availibility on
your system of an implementation of the  Message Passing
Interface (MPI)   if enabling the MPI  build  of  HPCG,
and a compiler that supports OpenMP syntax.

An implementation compliant with MPI version 1.1 is 
sufficient.

## Installation ##

The HPCG-NVIDIA software package is provided in a self-
contained  NVIDIA NGC container collection with all de-
pendencies provided.

## Valid Runs ##

See the file RUNNING in this directory.

HPCG   can be run in just a few   minutes from start to
finish.  However,  official runs must be at least  1800
seconds (30 minutes) as reported in the output file.
The Quick Path option is an exception for machines that
are in production mode  prior to broad  availability of
an optimized version of HPCG 3.0 for a given platform.
In this situation (which should be confirmed by sending
a  note  to the HPCG Benchmark owners)  the  Quick Path
option can be invoked by setting the run time parameter
equal to 0 (zero).

A valid run must also execute a problem size that is lar-
ge enough so that data arrays accessed in the CG iterati-
on loop do not fit in the cache of the device  in a way 
that would be unrealistic in a real application setting.
Presently this restriction means that the  problem size
should be large enough to occupy a significant fraction
of *main memory*, at least 1/4 of the total.

Future  memory system architectures may require restate-
ment of the specific memory size requirements.  But the
guiding principle will always be that the  problem size 
should reflect what would be reasonable for a real spa-
rse iterative solver.

## Tuning ##

See the file `TUNING` in this directory.

## Further information ##

Check out  the website  http://www.hpcg-benchmark.org/
for the latest information and performance results.


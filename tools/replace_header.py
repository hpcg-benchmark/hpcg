#! /usr/bin/env python

"""
Script to test and replace header in all source and header files.

Typical invocation:

python tools/replace_header.py src/*.[hc]pp testing/*.cpp
"""

import sha

HEADER = """//@HEADER
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
//@HEADER"""

def replace_header(filename):
  global HEADER

  comment = "//@HEADER"

  fobj = open(filename)
  data = fobj.read()
  fobj.close()

  begin_idx = data.find(comment)
  end_idx = data.find(comment, begin_idx+1) + len(comment)
  print begin_idx, end_idx-begin_idx, sha.sha(data[begin_idx:end_idx]).hexdigest(), filename

  if data[begin_idx:end_idx] != HEADER:
    fobj = open(filename, "w")
    fobj.write(data[:begin_idx] + HEADER + data[end_idx:])
    fobj.close()

def main(argv):
  for filename in argv[1:]:
    replace_header(filename)
  return 0

if "__main__" == __name__:
  import sys
  sys.exit(main(sys.argv))

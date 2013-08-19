#! /usr/bin/env python

import os
import re

def generate_rule(fname):
  if fname[-4:] != ".cpp":
    return

  base = fname[:-4]

  regex = re.compile(r'#\s*include\s*"([\w/.]+)"')

  deps = list()

  for fline in open(fname):
    mtch = regex.search(fline)
    #print mtch, fline
    if mtch:
      deps.append(mtch.group(1))

  print "%s.o:" % base, fname,
  for d in deps:
    print d,
  print "\n"

def usage(argv0):
  print "Usage:\n", argv0, "[--prefix=path]"

def main(argv):

  prefix = ""
  for a in argv[1:]:
    if a.startswith("--help") or a == "-h":
      usage(argv[0])
      break

    elif a.startswith("--prefix="):
      prefix = a[a.find("=")+1:]

    else:
      generate_rule(a)

  return 0

if "__main__" == __name__:
  import sys
  sys.exit(main(sys.argv))

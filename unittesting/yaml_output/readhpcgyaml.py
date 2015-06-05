
import dircache
import sys

import yaml

def read_yaml_files(filenames):
    for filename in filenames:
        if filename[-4:] == "yaml":
            yaml.load(open(filename).read())

def main(argv):
    if len(argv) > 1:
        read_yaml_files(argv[1:])
    else:
        read_yaml_files(dircache.listdir("."))

    return 0

if "__main__" == __name__:
    sys.exit(main(sys.argv))

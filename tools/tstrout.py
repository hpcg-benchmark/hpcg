#! /usr/bin/env python
# -*- encoding: utf8 -*-

from __future__ import print_function

import os
import sys
import re

TOKEN_BASE0 = 10000
TOKEN_NAME     = TOKEN_BASE0 + 1
TOKEN_SPACE    = TOKEN_BASE0 + 2
TOKEN_OPERATOR = TOKEN_BASE0 + 3

def tokenize(expr_or_stmt):
    tokens_regexps = [
        (TOKEN_SPACE, re.compile("[ \t\n\r]+") ),
        (TOKEN_NAME,  re.compile("[_A-Za-z][_A-Za-z0-9]*") ),
        (TOKEN_OPERATOR,  re.compile("[-+*/&|]") ),
    ]

    tokens = list()

    working_val = expr_or_stmt[:]

    while len(working_val) > 0:
        print(working_val, len(working_val))
        for tok, regexp in tokens_regexps:
            matched = regexp.match(working_val)
            if matched is None:
                continue

            if tok != TOKEN_SPACE:
                tokens.append((tok, working_val[matched.start():matched.end()]))

            working_val = working_val[matched.end():]

            if tok == TOKEN_SPACE:
                #print("Current length", len(working_val))
                continue

            break

            print(matched, dir(matched))
            if matched.pos == 0:
                print(matched.endpos)
                print(matched.pos)
                print(matched.expand)

        else:
            raise ValueError, repr(working_val)

    return tokens

def cplusplus_dtype(arg):
    tokens = tokenize(arg)
    #print(tokens)

    pre_all = tokens[0][1]
    name = tokens[-1][1]

    if len(tokens) == 2:
        pre_each, post_each = "", ""
    elif len(tokens) == 3:
        pre_each = tokens[1][1]
        post_each = ""
    else:
        raise ValueError, repr(tokens)

    return ((pre_all, pre_each, post_each), name)

SHIFTWIDTH = 4

def code_write(fd, indent, s):
    fd.write(indent*SHIFTWIDTH*" " + s)

def code_include(fd, filename, system_header=False):
    if system_header:
        ch_open  = "<"
        ch_close = ">"
    else:
        ch_open = '"'
        ch_close = '"'

    fd.write("#include " + ch_open)
    fd.write(filename)
    fd.write(ch_close + "\n\n")

def generate_tester(cplusplus_filename):
    directory, fname = os.path.split(cplusplus_filename)
    routname, extension = fname.split(".")

    #
    # read C++ file with implementation
    fd = open(cplusplus_filename)
    data = fd.read()
    fd.close()

    # find begining of arguments' string
    dfn_loc = data.find(routname + "(")
    dfn_open_brace_loc = data.find("{", dfn_loc)

    # find end of arguments' string
    idx = data.find(")", dfn_loc, dfn_open_brace_loc)
    while idx > 0:
        new_idx = data.find(")", idx+1, dfn_open_brace_loc)
        if new_idx == -1: # no other ")" found or the end reached
            break
        if new_idx > 0: # another ")" found
            idx = new_idx

    args_str = data[dfn_loc + len(routname) + 1:idx].strip()
    #print(args_str)

    # parse arguments' string
    args = list()
    for arg in args_str.split(","):
        dtype, name = cplusplus_dtype(arg)
        args.append((dtype, name))

    # tester routine source file
    tester_filename = "t_" + routname + "." + "cc"
    tpath = os.path.join(directory, tester_filename)
    tfd = open(tpath, "w")

    # system headers
    code_include(tfd, "cstring", True)
    code_include(tfd, "cstdio", True)

    # tester routine header
    header_filename = os.path.join(directory, routname + ".hpp")
    if os.path.exists(header_filename):
        code_include(tfd, routname + ".hpp")

    indent = 0

    # help routine
    code_write(tfd, indent, "void\nshow_help(const char *progname) {\n")
    indent += 1

    code_write(tfd, indent, 'printf("Tester for {}() function with the following arguments:\\n");\n'.format(routname))
    for arg in args:
        code_write(tfd, indent, "printf({}--{}=<{}>\\n{});\n".format('"', arg[1], arg[0], '"'))

    indent -= 1

    code_write(tfd, indent, "}\n")

    # main() function starts here
    code_write(tfd, indent, "int\nmain(int argc, char *argv[]) {\n")

    indent += 1

    # declare each argument
    for arg in args:
        # test for passed-by-reference: don't print "&" for these arguments
        code_write(tfd, indent, "{} {}{}{}".format(arg[0][0], arg[0][1] if arg[0][1] != "&" else "", arg[1], arg[0][2]))
        if arg[0][1] == "*":
            tfd.write(" = new {}()".format(arg[0][0])) # call default constractor
        tfd.write(";\n")


    # if not arguments given, then show help text (cannot use --help because there might be argument called help)
    code_write(tfd, indent, "if (argc <= 1 || argv[1] == NULL) {\n")
    code_write(tfd, indent+1, "show_help(argv[0]);\n")
    code_write(tfd, indent, "}\n")

    # loop over command line arguments
    code_write(tfd, indent, "for(int i=1; i<argc && argv[i]; ++i) {\n")

    indent += 1

    for arg in args:
        if arg[0][0] in ("int", "local_int_t"):
            cmd_arg = "--" + arg[1] + "="
            code_write(tfd, indent, "if (strncmp(argv[i], {}{}{}, {})==0) {}\n".format('"', cmd_arg, '"', len(cmd_arg), "{"))

            indent += 1
            # parse the value of the command argument and store it the C++ variable
            code_write(tfd, indent, "if (sscanf(argv[i]+{}, \"%d\", &{}) != 1)\n".format(len(arg[1])+2+1, arg[1]))

            indent += 1
            # assign 0 to the C++ variable
            code_write(tfd, indent, "{} = 0;\n".format(arg[1]))
            indent -= 1

            indent -= 1

            code_write(tfd, indent, "}\n")

    indent -= 1

    code_write(tfd, indent, "}\n")

    code_write(tfd, indent, "{}(".format(routname)) # start the call to the tested function

    sep = ""
    for arg in args:
        tfd.write(sep)
        tfd.write(arg[1])
        sep = ","

    tfd.write(");\n") # end the call to the tested function

    for arg in args:
        if arg[0][1] == "&": # if this is an argument passed by reference
            code_write(tfd, indent, "printf(\"{}=%g\\n\", (double){});\n".format(arg[1], arg[1]))

    code_write(tfd, indent, "return 0;\n") # return 0 from main()

    indent -= 1

    code_write(tfd, indent, "}\n") # end of main()

    tfd.close()

def main(argv):
    generate_tester(argv[1])
    return 0

if "__main__" == __name__:
    sys.exit(main(sys.argv))

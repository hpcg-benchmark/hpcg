"""
Copyright (c) 2002-2022 the Network-Based Computing Laboratory
(NBCL), The Ohio State University.

Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)

For detailed copyright and licensing information, please refer to the
copyright file COPYRIGHT in the top level OMB directory.
"""

import argparse


def get_parser():
    parser = argparse.ArgumentParser(
        description='OMB-Py Script',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--benchmark', type=str,
                        help='Name of benchmark to run')
    parser.add_argument(
        '--buffer', type=str, default='',
        help='Buffer type to be used in benchmark')
    parser.add_argument(
        '--pickle', action='store_true', default=False,
        help='Use mpi4py pickle methods')
    parser.add_argument('--min', type=int, default=None,
                        help='Minimum message size')
    parser.add_argument('--max', type=int, default=None,
                        help='Maximum message size')
    parser.add_argument('--skip', type=int, default=None,
                        help='Number of warmup iterations')
    parser.add_argument(
        '--iterations', type=int, default=None,
        help='Number of main iterations')
    return parser

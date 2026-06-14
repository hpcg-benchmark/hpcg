"""
Copyright (c) 2002-2022 the Network-Based Computing Laboratory
(NBCL), The Ohio State University.

Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)

For detailed copyright and licensing information, please refer to the
copyright file COPYRIGHT in the top level OMB directory.
"""

class Options:
    def __init__(self, benchmark_name, args):
        self.args = args
        self.iterations = 10000
        self.iterations_large = 100
        self.skip = 1000
        self.skip_large = 10
        self.min_message_size = 1
        self.max_message_size = 1 << 20
        self.large_message_size = 8192
        self.pickle = self.args.pickle
        self.buffer = None
        self.benchmark = benchmark_name
        self.update_options()

    def update_options(self):
        pt2pt = {'latency', 'bw', 'bibw', 'multi_lat'}
        coll_reduce = {"reduce", 'allreduce', 'reduce_scatter'}

        if self.args.buffer:
            self.buffer = self.args.buffer
        if self.args.iterations:
            self.iterations = self.args.iterations
            self.iterations_large = int(self.args.iterations/100)+1
        elif(self.args.benchmark in {'bw', 'bibw'}):
            self.iterations = 100
            self.iterations_large = 30
        if self.args.skip:
            self.skip = self.args.skip
            self.skip_large = int(self.args.skip/100)+1
        elif(self.args.benchmark in {'bw', 'bibw'}):
            self.skip = 10
            self.skip_large = 3
        if self.args.max:
            self.max_message_size = self.args.max
        elif(self.args.benchmark in pt2pt):
            self.max_message_size = 1 << 22
        if self.args.min:
            self.min_message_size = self.args.min
        elif(self.args.benchmark in {'latency', 'multi_lat'}):
            self.min_message_size = 0
        elif(self.args.benchmark in coll_reduce):
            self.min_message_size = 4

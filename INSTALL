####################################################
High Performance Conjugate Gradient Benchmark (HPCG)
####################################################

:Author: Jack Dongarra and Michael Heroux and Piotr Luszczek
:Revision: 3.1
:Date: March 28, 2019

=================
Clone from GitHub
=================
The official HPCG repository is on GitHub.com.  You may fork the
repository from here::

https://github.com/hpcg-benchmark/hpcg

===================
Download and Unpack (only if you did not clone from GitHub.com)
===================

Retrieve the tar-gzip file, from http://hpcg-benchmark.org

then uncompress it with::

    gunzip hpcg.tar.gz; tar -xvf hpcg.tar

this  will create  an ``hpcg`` directory, that  we call  the top-level
directory in the text below.


=============
Configuration
=============

HPCG provides three methods of configuration: one based on make(1),
one based on CMake, and one based on Autoconf. They are discussed in
more details below.

---------------------------
Configuration based on Make
---------------------------

HPCG configuration with a make-compatible file for configuring the
compilers, linkers, and their command line flags starts with creating a
file ``Make.<arch>`` in the ``setup`` directory right under the
top-level  directory. The suffix ``<arch>`` should be a name that you
choose for your system's configuration. There may be multiple
configuration, each with a different set of options and flags selected,
for example: ``setup/Make.mysys1``, ``setup/Make.mysys2``, and so on.
To make the process easier, you may want to re-use one of the example
files already provided in the ``setup`` directory.  The ``Make.<arch>``
file contains the command names of the compilers and linkers together
with their options for using special directories and linking with
libraries as well as their paths that are to be used for building the
binary.

The most important variables that need to be set in ``Make.<arch>``
are::

* ``MPdir`` specifies the path to where the MPI installation resides.
  For some MPI implementations, using the MPI-enabled compiler (such as
  ``mpicxx``) is sufficient and it is not required specified any MPI
  installation paths. This variable can be used to setup the ``MPinc``
  and ``MPlib`` variables.

* ``MPinc`` specifies the paths to include directories with MPI header
  files. A common setting here would be ``MPinc = -I$(MPdir)/include``,
  provided that the ``MPdir`` variable was set properly.

* ``MPlib`` specifies the path to directories with MPI library files. A
  common setting here would be ``MPlib = $(MPdir)/lib``, provided that
  the ``MPdir`` variable was set properly.

* ``HPCG_OPTS`` specifies additional compiler options for compiling the
  HPCG source files. These options might include optimization,
  debugging, or tuning flags.

* ``CXX`` specifies the name of the C++ compiler for compiling the code.
  It should be a C++ that supports a subset of STL that is used in HPCG
  as well as the subset of constructs used in the MPI standard.

* ``CXXFLAGS`` specifies the flags to be used when invoking the C++
  compiler.  These might include optimization and profiling options.

* ``LINKER`` specifies the linker capable of linking C++ programs that
  include MPI calls.

* ``LINKERFLAGS`` specifies the flags passed to the linker that might
  indicate directories where to look for standard C++ and MPI libraries.

HPCG supports in-source and out-of-source builds. In the case of the
former, the object files and the binary reside in the same directory
structure as the source files. There is no need for a separate
configuration step and the reader may skip the the Build section.

In the latter case, a separate directory is created to hold the object
and binary executable files.  For example, create a custom directory,
``build`` in this example, for the results of compilation and linking::

    mkdir build

Next, go this new directory and use the ``configure`` script to create
the build infrastructure::

    cd build
    /path/to/hpcg/configure <arch>

Notice that you have to give an absolute or relative path to the
``configure`` shell script so that the location of the source files can
be discovered automatically.

<arch> is the suffix of the setup file you created in the directory
hpcg/setup.  Sample setup files are found in the hpcg/setup directory.

----------------------------
Configuration based on CMake
----------------------------

For configuration based on CMake, the compiler and linker are specified
with the CMake-specific flags. Additionally, HPCG-specific options of
interest include::

* ``HPCG_ENABLE_DEBUG`` option enables a build with debugging output
  (``OFF`` by default).

* ``HPCG_ENABLE_DETAILED_DEBUG`` option enables a build with detailed
  debugging output (``OFF`` by default).

* ``HPCG_ENABLE_MPI`` option enables a build with MPI enabled (``OFF``
  by default).

* ``HPCG_ENABLE_LONG_LONG`` option enables a build with ``long long``
  type used for global indices (``ON`` by default).

* ``HPCG_ENABLE_OPENMP`` option enables a build with OPENMP enabled
  (``OFF`` by default).

There are more options available. Please see the ``CMakeLists.txt`` file
and the ``option()`` commands therein.

These options may be changed with ``ccmake`` command or given directly
to the ``cmake`` invocation with the ``-D`` prefix.

-------------------------------
Configuration based on Autoconf
-------------------------------

It is possible to configure HPCG using Autoconf and Automake (part of
the GNU Autotools collection). In order to generate the configuration
script, use the following command::

    autoreconf -ivf

this will overwrite the `configure` script that ships with HPCG. The new
script can then be used through a standard invocation process and
command line options that correspond to the ones available to the CMake
configuration.

=====
Build
=====

For in-source builds, while in the top-level directory, simply type::

    make path/to/setup/file

For the out-of-source builds, go to the ``build`` directory and type
``make``.

In both cases, the executable called ``xhpcg`` should be created in the
``bin`` directory.

Building after CMake-based configuration depends on which build tool was
selected with the ``-G`` option.


=============
Example Build
=============

As an example, let's use a Linux cluster and create a file called
``Make.Linux`` in the ``setup`` directory right under the top-level
directory.

For in-source build, we type ``make setup/Make.Linux`` which creates the
executable file called ``bin/xhpcg``.

For out-of-source build, we create the build directory called
``build_Linux`` and go to that directory::

    mkdir build_Linux
    cd build_Linux

Then, while in the ``build_Linux`` directory we type::

    /path/to/hpcg/configure Linux
    make

This creates the executable file ``bin/xhpcg``.


====
Test
====

For a quick check, go to the ``bin`` directory and run the HPCG
executable as follows::

    mpirun -np 8 xhpcg

Note that this will use the default ``hpcg.dat`` file. If you'd like to
change the size of local dimensions of the problem to ``NX=32``,
``NY=24``, ``NZ=16`` then run the following::

    mpirun -np 8 xhpcg 32 24 16

You can also specific size and runtime parameters using ``--nx``,
``--ny``, ``--nz``, ``--rt``.  For example, for specifying the local
grid dimensions to be NX=NY=NZ=16, and the timed phase execution limit
of 30 minutes (1800 seconds) you can use

    mpirun -np 4 xhpcg --nx=16 --rt=1800


======
Tuning
======

Most of the performance  parameters can be tuned by modifying the input
file ``hpcg.dat``. See the file ``TUNING`` in the top-level directory.


====================
Compile Time Options
====================

At the end of the "model" ``Make.<arch>``, the  user  is given the
opportunity to compile the software with some specific compile options.
The list of this options and their meaning are::

* Compile with modest debugging turned on::

    -DHPCG_DEBUG

* Compile with sparse matrix arrays allocated contiguously. This option
may be helpful on systems with pre-fetch::

    -DHPCG_CONTIGUOUS_ARRAYS

* Compile with voluminous debugging information turned on::

    -DHPCG_DETAILED_DEBUG

* Compile with MPI disabled::

    -DHPCG_NO_MPI

* Compile without OpenMP enabled::

    -DHPCG_NO_OPENMP

* Enable detail timers::

    -DHPCG_DETAILED_TIMING


By default HPCG will::

* Turn on MPI support.
* Turn on OpenMP support.
* not display detailed timing information.


===================
Further information
===================

Check out  the website http://hpcg-benchmark.org/ for the latest
information.

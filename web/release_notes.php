<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('Mantevo - Release Notes');
  $page->setNavIdentifier('download');
 
  $y = <<< END
    h2 { padding-top: 2em; }
END;
  $page->setInlineStyles($y);
  
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="download.php">Download</a> - Release Notes</div>

<h3> Mantevo Suite Release 1.0</h3>

<p>The first official release of the Mantevo suite of mini applications and
mini drivers is available for download.</p>

<p>Mantevo packages are small programs that embody one or more performance impacting elements of large-scale production applications.  A number of Mantevo packages have already been successfully used as part of the "co-design" of new computer systems and applications during this time of rapid transition to scalable multicore and accelerator based computer systems.</p>

<p>Mantevo Suite Release 1.0 includes eight packages, including seven Miniapplications: CloverLeaf, CoMD, HPCCG, MiniFE, MiniGhost, MiniMD, and MiniXyce, and one Minidriver: EpetraBenchmarkTest.</p>
 
<p>For more information, or to download these packages, visit the following links.</p>

<p>Mantevo website: <a href="http://mantevo.org">Mantevo.org</a><br/>
Mantevo Suite 1.0 <a href="http://mantevo.org/download.php">Download page</a>
</p>

<!--
<table cellspacing="0" cellpadding="0">
<tr><td><a href="#votd">Version of the Day</a></td><td></td></tr>
<tr><td><a href="#v4.1">Version 4.1</a></td><td>(9/21/07)</td></tr>
<tr><td><a href="#v4.0">Version 4.0</a></td><td>(5/12/06)</td></tr>
<tr><td><a href="#v3.3">Version 3.3</a></td><td>(12/23/04)</td></tr>
<tr><td><a href="#v3.2">Version 3.2</a></td><td>(7/21/04)</td></tr>
<tr><td><a href="#v3.1.2">Version 3.1.2</a></td><td>(7/09/03)</td></tr>
<tr><td><a href="#v3.1.1">Version 3.1.1</a></td><td>(5/21/03)</td></tr>
<tr><td><a href="#v3.1">Version 3.1</a></td><td>(4/04/03)</td></tr>
<tr><td><a href="#v3.0">Version 3.0</a></td><td>(3/30/02)</td></tr>
<tr><td><a href="#v3.0b">Version 3.0 beta</a></td><td>(11/29/01)</td></tr>
</table>

<a name="votd"></a><h2>Version of the Day</h2>

<p>Since the last major release (v4.1 on 9/21/07), the following enhancements
are available in the "version of the day" (VOTD) releases:</p>

<ul>
<li>No release notes available at this time.</li>
</ul>

<a name="v4.1"></a><h2>Version 4.1 (9/21/07)</h2>

<h3>Uncertainty Quantification (UQ) Capabilities:</h3>

<ul>
<li><em>New method:</em> Wiener-Askey generalized polynomial chaos expansions
approximate the functional relationship between response metrics and their
underlying random variables.  Orthogonal polynomial bases include Hermite,
Legendre, Laguerre, Jacobi, and generalized Laguerre.  Expansion coefficients
are estimated by sampling, tensor-product quadrature, sparse grid cubature
(using Clenshaw-Curtis), or linear regression.  Additional features include
analytic response moments, analytic random variable sensitivities, and analytic
design variable sensitivities.  Refer to Section 6.4 of the <a
href="./Users4.1.pdf">Users Manual</a> for additional information.</li>

<li><em>New method:</em> Efficient Global Reliability Analysis (EGRA) employs
mean and variance estimates from Gaussian Process (GP) models to form an
expected feasibility function (EFF), which is then maximized using a global
optimizer to select points for GP refinement.  This process accurately
estimates response function contours, for which probabilities are then computed
using multimodal adaptive importance sampling.  Refer to Section 6.3.3 of the
<a href="./Users4.1.pdf">Users Manual</a> for additional information.</li>

<li><em>New method:</em> incremental Latin Hypercube and Monte Carlo sampling.
In the incremental mode, one augments an initial LHS/MC sample set of size N
with a second sample set of size N.  Both samples together (of size 2N) will
preserve the stratification and correlation structure of the original
sample.</li>

<li><em>New method:</em> importance sampling (standard, adaptive, and
multimodal adaptive) added for local and global reliability methods.</li>

<li>Second-order integrations corrected (negative reliability indices and
Epetra symmetric matrix products) for use within local reliability
methods.</li>

<li>All UQ methods extended to support generalized reliability indices.</li>

<li>Epistemic methods now support final statistics results for use in
optimization under uncertainty (OUU).  Refer to the Models chapter in the
Reference Manual for additional information on epistemic interval results
accessible within OUU and to Section 10.6.2 of the <a
href="./Users4.1.pdf">Users Manual</a> for initial guidance on formulating
epistemic OUU studies.</li>

<li>Added compile time switches for Tausworthe and Mersenne twister random
number generators for use with LHS/MC.  These are not yet the default, as
performance testing is ongoing.</li>

<li>Variance-based decomposition and Dempster-Shafer theory of evidence now
support multiple response functions.</li>
</ul>

<h3>Optimization Capabilities:</h3>

<ul>
<li>The multilevel hybrid optimization strategy now supports flow of multiple
solutions between iterators.  Iterators may now accept one or multiple starting
point and may output one or multiple final results.  The strategy logic
maintains a consistent data flow and supports concurrency in multiple iterator
instances as required (i.e., multiple data points require multiple instances of
an iterator that accepts only a single input).</li>

<li>the surrogate-based optimization (sbo) strategy now supports multipoint
approximations (i.e., tana-3), approximate subproblem reformulations for
nonlinearly-constrained problems (subproblem objective may be the original
surrogate objective, a surrogate lagrangian, or a surrogate augmented
lagrangian; subproblem constraints may be the original surrogate constraints,
linearized surrogate constraints, or no constraints), and constraint relaxation
using a heuristic homotopy approach.  in addition, previous augmented
lagrangian updating approaches have been replaced with the provably-convergent
approach of conn, gould, and toint.  see section 9.6 of the <a
href="./users4.1.pdf">users manual</a> for formulations and <a
href="../papers/aiaa-2006-7117.pdf">aiaa mao conference paper</a> for
computational results.

<li><em>new strategy:</em> a global surrogate-based optimization strategy has
been added.  rather than being localized using trust-regions, each approximate
optimization cycle identifies points for continuing updates to a single global
surrogate model.  this is most appropriate for use with interpolatory
surrogates, such as kriging/gaussian process models.  

<li><em>new ouu approach:</em> polynomial chaos-based design optimization
(pc-bdo) is now supported in a bi-level formulation employing one of three
different design sensitivity approaches: analytic design gradients based on a
pce formed over all variables, numerical design gradients based on a pce formed
only over the random variables, and numerical design gradients based on a pce
formed over all variables. in the cases where the expansion is formed over all
variables, only a single pce construction is required for the complete pc-bdo
process, whereas the expansions only over the random variables must be
recomputed for each change in design variables.  refer to section 10.6.2 of the
<a href="./users4.1.pdf">users manual</a> for additional information on design
sensitivity analysis formulations.

<li><em>New method:</em> the NCSU implementation of DIRECT by Joerg Gablonsky
has been added as a global optimizer, augmenting the existing COLINY
implementation of DIRECT.  Additional NCSU optimizers, especially implicit
filtering, will be added in time.

<li><em>New method:</em> closely related to the new EGRA uncertainty
quantification method described above, the Efficient Global Optimization (EGO)
method has been added.  It employs mean and variance estimates from Gaussian
Process (GP) models to form an expected improvement function (EIF), which is
then maximized using a global optimizer to select points for GP refinement.
This process balances the desires to exploit areas with good solutions and
explore areas with limited data, resulting in excellent global search
characterisitcs.  This method supports nonlinear constraints through an
augmented Lagrangian merit function formulation.

<li><em>New method plug-ins:</em> Support for dynamic optimizer plug-ins has
been added.  This initially targets SNOPT and NPSOL as shared object libraries,
but will in time allow more flexible binding to commercial solvers at run time
(currently managed at configure/make time).

<li>OPT++ now supports an internal mode override, allowing more effective
combination of optimizer evaluation requests, as well as iteration reset
functionality, allowing an instance to be instantiated once and reused multiple
times.

<li>Update to the DAKOTA/AMPL interface to handle algebraic nonlinear
constraints using the AMPL specification. This update allows the use of
standard AMPL model files without modification of the constraint specification.
</ul>

<h3>Nonlinear Least Squares Capabilities:</h3>

<ul>
<li>Calculation of residuals can now be simplified through user specification
of a least squares data file, which provides the set of data to which a
computational model will be calibrated.</li>
</ul>

<h3>DACE Capabilities:</h3>

<ul>
<li>Initial interface to PSUADE from LLNL, providing access to the Morris
One-At-a-Time (MOAT) method.</li>
</ul>

<h3>Framework Enhancements:</h3>

<ul>
<li>DAKOTA input files no longer require the use of "\" for line continuations.
All examples have been updated to omit the backslash.</li>

<li>Gaussian process surrogate models now support variance estimators for use
in expected improvement/expected feasibility functions; constant, linear, or
quadratic trend functions; global optimization methods for maximum likelihood
estimation; and point filtering.</li>

<li><em>New derived Model class:</em> RecastModel.  This is a thin wrapper
around a sub-model allowing for linear and nonlinear recasting of its variables
and responses.  Used by variable and response scaling, random variable
transformations, multiobjective optimization, merit functions (see SBO
discussion above), and expected improvement/feasibility, as described in
Section 10.3 of the <a href="./Users4.1.pdf">Users Manual</a>.</li>

<li>Extensions to variables views in surrogate model construction.  Global
surrogate classes previously assumed a hardwired "All" view (due to the use of
this view by DACE methods) and local/multipoint/hierarchical surrogates
previously assumed a hardwired "Distinct" view.  Now general view support is
available in all surrogate types, which allows much broader surrogate
deployment than was previously possible.  As part of this extension, sampling
methods now support additional sampling modes (ACTIVE, ACTIVE_UNIFORM, ALL,
ALL_UNIFORM, UNCERTAIN and UNCERTAIN_UNIFORM) and many classes have been
extended to support data translations among differing variables set views.</li>

<li>Methods may now utilize mixed surrogate/truth models, as identified by a
surrogate identifier list.  The initial use case is sequential RBDO.</li>

<li>More flexible interface to input/output scaling and added support for
logarithmic scaling.  In addition, numerical derivative estimation is now
performed within the scaled space to avoid step size thresholds.</li>

<li>Extended instantiate "on-the-fly" support for Iterators, Models,
Interfaces, Approximations, and Constraints in to support advanced model
recursions in local/global reliability and PCE methods.</li>

<li>Support for "NaN" and +/-"Inf" added to the input parser (for use in
failure recovery values) and the response file reader.  For both cases,
mitigation approaches are iterator-specific.  Currently, sampling methods will
omit samples that are not finite from their summary statistics.  See Section
20.4 of the <a href="./Users4.1.pdf">Users Manual</a> for additional
information.</li>

<li><em>New interfaces:</em> a new direct interface to Python has been
developed and the existing direct interface to Matlab has been refined.</li>

<li>Extended surrogate build/update/append functionality within Models,
Interfaces, and Approximations.</li>

<li>Added representation access function for letter/envelope idiom to reduce
proliferation of virtual functions at envelope base classes.</li>
</ul>

<h3>Miscellaneous:</h3>

<ul>
<li>DAKOTA now supports parallel make for concurrent compilation.</li>

<li>Ports to MINGW for Windows are now supported by the autotools harness.</li>

<li>Revision control for DAKOTA is now managed by subversion, allowing
directory reorganization/renaming, upgrade of our release processes to employ
release branches, and upgrade of our TPL processes to employ svn
externals.</li>

<li>Stable releases have been added as new developmental releases (supplements
existing VOTD release mechanisms), and revision numbers are now embedded within
all developmental releases for traceability.</li>

<li>The nightly test harness now includes coverage of restart capabilities, as
required for incremental sampling methods.</li>

<li>Bug fixes (Bugzilla record id): reliability methods (#1327, #2799), SBO
(#2124, #2829, #2836), RBDO (#2828, #2832), PLplot window advance (#2180),
sampling bounds (#2824), COLINY interface (#2250, #2434), JEGA parallelism
(#1264), parallel usage of plug-in interfaces (#2826, #2827), parallel usage of
instantiate on-the-fly Models (#2834), label propagation through Model
recursions (#2831), IEEE special values in iostreams (#2855), derivative
estimation in recast models (#2835), library build dependencies (#2158, #2822,
#2843), and VOTD distribution process (#2841).</li>
</ul>

<a name="v4.0"></a><h2>Version 4.0 (5/12/06)</h2>

<h3>Autotools</h3>

<ul>
<li>Upgrade from Cygnus configure to the latest GNU autotools (e.g., autoconf,
automake). This renders the configure/make system much more scalable to
emerging platform build targets. It moves from a host/target-based approach to
a feature-based approach, where configure probes are used to detect available
compiler features. The 4.0 release supports our primary platform targets (Red
Hat, Solaris, AIX, IRIX, and OSF). Continuing refinements to the new
configuration system are planned, and future VOTD and interim releases will
support Mac OSX, Cygwin, and mingw, among others.</li>
</ul>

<h3>JAGUAR and AMPL</h3>

<ul>
<li>A JAVA front end for DAKOTA (JAGUAR) is now available.  At the time of
DAKOTA v4.0 release, JAGUAR is in 1.0 beta, but the basics of defining a DAKOTA
input file are available.  Future enhancements for graphical simulation
interfacing and visualization of results are planned.</li>

<li>As part of supporting the JAGUAR equation editor capability, DAKOTA now
supports an interface to the AMPL solver library. This allows for the use of
algebraic problem definitions to define any subset of an Interface's mapping of
Variables to Responses.  The <tt>algebraic_mappings</tt> keyword specifies file
names that define AMPL expression graphs, and the AMPL solver library processes
the files to perform the algebraic mappings as well as evaluate any
derivatives.</li>
</ul>

<h3>Framework enhancements:</h3>

<ul>
<li>Addition of <a href="./votd/html-ref/ModelCommands.html">"model"</a> as a
sixth keyword specification block. This increases flexibility in model
recursion for local and hierarchical surrogates (the previous approach of
embedding the model recursion controls within the method specification was too
restricting since no method is used in the construction of these surrogates).
This specifically targets multifidelity and local Taylor-series based
surrogate-based optimization where the surrogate models involve additional
recursions (e.g., additional data fits or uncertainty analyses). This change is
backwards compatible for simple (i.e., single method) input files, but is
incompatible for more advanced multi-method input files such as SBO, OUU, RBDO,
etc.</li>

<li>Migration of the previous approximation interface specification into the
new surrogate model specification. This allows for the consolidation of
previous layered model/approximation interface specification pairs (present in
many SBO input files) into new surrogate model specifications. As there is no
longer an application/approximation selection in the interface keyword, the
application keyword has also been removed. </li>

<li>Active set generalization: application interfaces are now passed an
additional derivative values vector (DVV) to specify the required derivative
components. This is important for analytic gradients in optimization under
uncertainty (see RBDO item below), but unfortunately, also defines a change in
parameters file format which is <em>not backwards compatible</em>. </li>

<li>Addition of <tt>analysis_components</tt> to system and fork application
interfaces. Again, this required extension of the parameters file format. Also
consolidated the previous <tt>modelcenter_file</tt> specification within this
new specification. </li>

<li>Revised dependency management to allow broader usage of global utilities.
This particularly improves error handling in templated data types.</li>

<li>Introduced the use of <tt>std::set&lt;&gt;</tt> and
<tt>std::map&lt;&gt;</tt>.  With the removal of support for some old C++
compilers, these STL constructs have replaced many previous uses of
<tt>std::list&lt;&gt;</tt> for evaluation bookkeeping.</li>

<li>Modified the creation mechanisms for
Iterator/Model/Variables/Interface/Responses objects to cache new object
creations and reuse them where appropriate.  This reduces memory consumption in
multi-Iterator/multi-Model strategies, and also supports evaluation counter
persistence in reused Models and Interfaces. It does, however, require more
sophisticated management of parallel configurations within the Model classes
(since a single model may now be used in multiple parallel contexts).
Iterators and Strategies now manage evaluation counters with explicit control
over absolute or relative counts.</li>

<li>Expanded the problem description database class into a class hierarchy,
with separation of generic parser operations and IDR-specific parser
operations.  An XML database (using Xerces plus the JAGUAR XML Schema) is
envisioned for eventual addition to the hierarchy.</li>

<li>Added support for a much finer grain of evaluation reporting.  This is
needed for algorithmic performance analysis and is activated only for
<tt>output</tt> = <tt>verbose</tt> or <tt>debug</tt>.  Value/gradient/Hessian
evaluation counters are used for each response function. This supplements the
previous counters which only track total/new calls to Interface::map() and do
not track the particular data requests for each call.</li>

<li>Library mode extensions: addition of new model/interface access functions,
repair of model recursion access functions for altering envelope data, repair
of parallel configuration initialization for interface plug-ins, addition of
parallel configuration option for splitting intra-communicators down to a
single processor (<tt>COMM_SPLIT_TO_SINGLE</tt>), and development of a sample
interface plug-in class.</li>
</ul>

<h3>Iterative methods</h3>

<ul>
<li>Update to JEGA v2.0, which adds convergence and niche pressure operators
for MOGA, improved and configurable user output, improvements to core data
structures, and numerous bug fixes.</li>

<li>Update to the latest version of OPT++, which adds namespace support and
upgrades to NEWMAT11.</li>

<li>Update to the latest version of DDACE, which adds support for
<tt>oa_lhs</tt> and one-way analysis of variance (ANOVA).</li>

<li>SGOPT methods have been deprecated and replaced with new COLINY methods.
Most COLINY methods support general nonlinear constraints, and the COLINY GA
supports mixed integer design variables.</li>

<li>Consolidation of specification (where possible) between JEGA and COLINY
genetic algorithms.</li>

<li>All optimizers and least squares methods now support scaling in variables,
linear constraints, and nonlinear constraints.  Scaling may be user-specified
characteristic value, by 2-sided bounds, or by 1-sided bounds.</li>

<li>Second-order integrations, second-order limit state approximations,
two-point adaptive nonlinear limit state approximations (TANA-3), and
second-order mean estimations are now supported for both RIA and PMA
reliability methods. Second-order PMA uses a backtracking Newton solve to
update the PMA reliability target given a second-order probability target.
Hessians may be analytic, numerical, or based on quasi-Newton updates.</li>

<li>The uncertain variable specification has been extended to include new
triangular, beta, gamma, gumbel, frechet, and interval distributions. The
reliability methods have been extended to support gamma, gumbel, frechet,
weibull, uncorrelated loguniform, uncorrelated triangular, and uncorrelated
beta distributions, and the nondeterministic sampling methods have been
extended to include beta, gamma, and triangular distributions.</li>

<li>An initial sampling-based implementation of Dempster-Shafer theory of
evidence is now available.</li>

<li>An interface to NLPQLP has been added. Thanks to Bengt Abel at BMW for
providing the initial version.</li>

<li>Granularity has been added to the virtual Iterator functions to support
additional abstract Iterator operations.  The previous construct/run/destruct
progression has been extended to a construct/pre_run/run/post_run/destruct
progression.</li>
</ul>

<h3>Model components</h3>

<ul>
<li>Previous versions of neural network, kriging, MARS, and regression
polynomials have been deprecated and replaced with the initial release of
Surfpack. Surfpack adds support for equality-constrained regression and will
provide radial basis function approximations in the future.</li>

<li>Multipoint approximations are now supported in the Approximation hierarchy.
The Two-point Adaptive Nonlinearity Approximation (TANA-3) has been added and
may be invoked either as a general approximation method or as a special limit
state approximation selection within reliability analysis.</li>

<li>Gaussian process global approximations are now supported in the
Approximation hierarchy. This class utilizes OPT++ to solve for the
approximation coefficients.</li>

<li>Approximation classes extended to support an anchor point, a special data
point for which exact matching is enforced (e.g., using equality-constrained
least squares).  Data requirement estimation is now more consistent, through
the estimation of the number of approximation coefficients, the number of
constraints, and the data content per sample.  Improved efficiency of
SurrogateDataPoint lists by adopting a reference counting design and
eliminating unnecessary data copying.</li>

<li>Migration of nonlinear constraint data from Iterator to Model in support of
strategies (e.g., new homotopy constraint-relaxation in surrogate-based
optimization) that modify nonlinear constraint bounds and targets. All
Iterators now pull constraint data at run time such that any Model updates are
properly captured.</li>

<li>Variables objects have been generalized to utilize a pair of variables
views, one for the active view and another for the inactive view.  This renders
the classes more general and more encapsulated from external logic.</li>

<li>Response objects are now dynamically resized when DVV lengths are updated.
Response objects now support derivatives with respect to either active or
inactive continuous variables in a more direct and efficient manner.
Derivative estimation and management in Model has been generalized to support
dynamic derivative needs based on the current content of the DVV.</li>

<li>Addition of a direct interface to the MATLAB engine. This currently
requires a dynamically linked DAKOTA executable.</li>
</ul>

<h3>Strategies</h3>

<ul>
<li>Reliability-based design optimization (RBDO) now supports analytic
sensitivities of reliability, probability, and response levels with respect to
design variables that either augment the uncertain variables or define
distribution parameters for the uncertain variables.  This enables
fully-analytic bi-level RBDO approaches. </li>

<li>The new model recursion and sensitivity capabilities described above enable
a new capability for sequential RBDO using a trust-region surrogate-based
approach. In this approach, reliability estimates and sensitivities are used to
form surrogate contraints based on Taylor series. This approach is the most
computationally efficient RBDO approach available in DAKOTA to date.
First-order and quasi-Newton-based second-order Taylor series are currently
supported.</li>

<li>The new model recursion facilities described above enable a new capability
for multifidelity surrogate-based optimization in which the low fidelity model
involves a data fit surrogate.</li>

<li>The surrogate-based optimization strategy has been extended to support
iterate acceptance via a filter method, trust region ratio calculation via an
augmented Lagrangian merit function, constraint relaxation via a homotopy
method, and hard convergence assessment via nonnegative least squares Lagrange
multiplier estimation. Lawson &amp; Hanson least squares routines have been
added for these Lagrange multiplier estimations. SBO with global data fits has
been restructured to allow hard convergence checks prior to surrogate
construction and passing of anchor points as embedded consistency constraints
(this allows the surrogate to be constructed once per cycle with embedded
consistency, rather than initially built, updated with the center, and then
corrected).</li>
</ul>

<h3>Packages and Utilities</h3>

<ul>
<li>GSL updated to version 1.7.</li>

<li>Epetra updated to version 3.4.  This enables access to LAPACK serial dense
eigensolvers (used in second-order probability integrations).</li>

<li>PLplot updated to version 5.5.2.</li>

<li>DPrePro updated to support more flexibility in parameters file formats and
numerical field regular expression matching.</li>

<li>Extensions to the Perl regression testing tool
(Dakota/test/sqa/dakota_test.perl) to support individual test extraction and
execution.</li>

<li>A new Perl utility (Dakota/test/sqa/dakota_update_input.perl) is available
for upgrading old v3.x input files to v4.0.</li>

<li>DAKOTA now supports a "dry run" mode, in which all parsing is performed and
all objects are instantiated, but the analysis is not performed.  This is
selected using the new "-check" command line argument and is useful for
checking input files for errors prior to parallel queue submission.</li>
</ul>

<h3>Documentation</h3>

<ul>
<li>The DAKOTA Users Manual has been migrated from FrameMaker to LaTeX and is
now managed in Dakota/docs.  This simplifies the management of the Users Manual
and will promote more regular updating.</li>

<li>SQE process documents updated to version 2.0.</li>

<li>Addition of example files for utilizing multilevel parallelism with
concurrent mpiexec launches within a single PBS allocation.</li>
</ul>

<h3>Bug fixes</h3>

<ul>
<li>Resolution of Bugzilla ids 1044, 1260, 1263, 1264, 1265, 1266, 1340, 1341,
1351, 1364, 1391, 1398, 1445, 1882, 1893, 1894, 1914, 1962, 1965, 1968, 2013,
2145, 2146, and 2161.  A selected subset is listed below (emphasis on
resolution of reported 3.3 issues).</li>

<li>Graphics windows and tabular data are now properly updated for Iterators
which employ nonblocking synchronization (requires data caching until
evaluation sequences are complete).</li>

<li>Corrected bug when using secondary file tagging with fork interfaces.</li>

<li>JEGA now properly configures processor allocations for message-passing
parallelism.</li>

<li>Central composite and Box-Behnken designs of experiments, which are
inflexible in their supported sample size, now support improved logic for
managing user inputs, specification defaults, and minimum sample
requirements.</li>

<li>population_size may now be specified independently from initialization_type
in COLINY and JEGA genetic algorithms.</li>

<li>corrected SBO graphics and uninitialized additive correction arrays in the
case of a multiplicative to additive on-the-fly correction switch (performed
when numerical issues are encountered with multiplicative corrections).</li>

<li>corrected "random" keyword duplication in initialization_type and
replacement_type for SGOPT/COLINY GAs.</li>

<li>protected sample statistics calculations (standard deviations and
confidence intervals) for the case of only one sample.</li>
</ul>

<a name="v3.3"></a><h2>Version 3.3 (12/23/04)</h2>

<h3>Framework enhancements:</h3>

<ul>
<li>Revisions to the Iterator class hierarchy to bring together the 
    uncertainty quantification, parameter study, and design of experiments
    classes so that common statistical analysis routines can be shared.
    <a href="./release/html/classDakota_1_1Iterator.html">Iterator</a>
    is now divided into two branches,
    "<a href="./release/html/classDakota_1_1Minimizer.html">Minimizer</a>"
    (for optimization and nonlinear least squares) and 
    "<a href="./release/html/classDakota_1_1Analyzer.html">Analyzer</a>"
    (for uncertainty quantification, parameter studies, and design of 
    experiments).</li>

<li>Major redesign of multilevel parallelism classes to be more modular 
    and extensible (the number of parallelism levels is now open-ended) 
    and to allow multiple parallel configurations to coexist (previously,
    only a sequence of partitions could be used, one at a time).  This
    development was driven by two needs: (1) performing communicator 
    partitioning at construction time instead of run time (for simplifying
    parallel set-up for direct simulation interfaces), and (2) allowing more
    complete parallelism management in strategies which have more than one
    active model at a time (e.g., multifidelity SBO, see Strategies section
    below).  Features include:</li>
    <ul>

    <li>The new design uses an open-ended list of 
        <a href="./release/html/classDakota_1_1ParallelLevel.html">ParallelLevel</a>
        objects which collectively contain all of the communicator
        partitions.  This list is managed with a list of 
        <a href="./release/html/classDakota_1_1ParallelConfiguration.html">ParallelConfiguration</a>
        objects, each of which identifies the set of ParallelLevels which 
        define a particular multilevel parallel configuration.</li>
    <li>Additional encapsulation of message-passing routines through addition
        of more MPI wrappers.</li>
    <li>Addition of a component parallel mode (INTERFACE, SUBMODEL, or none)
        facility to the Model hierarchy to manage the Model component whose
        parallel configuration is currently active.
    </li></ul>

<li>Addition of finite-difference numerical Hessians and BFGS/SR1 
    quasi-Newton Hessians to DAKOTA's derivative estimation routines.  
    Previously, DAKOTA relied on vendor optimization algorithms to perform
    these calculations, but a native capability was needed for FD- and 
    quasi-second-order surrogate corrections.  Features include:</li>
    <ul>
    <li>Numerical Hessians may use 1st-order gradient differencing or 
        2nd-order function-value differencing.  This selection is 
        automatically determined based on the gradient specification: the
        former if analytic gradients and the latter otherwise.  FD Hessians
        from 1st-order gradient differences have a symmetry enforcement
        [H = 1/2 (H + H^T)]</li>
    <li>As for numerical gradients, the numerical Hessians support the
        ability to be fully asynchronous (i.e., multiple gradient/Hessian
        data sets may be evaluated concurrently, each of which involves
        multiple finite difference evaluations).</li>
    <li>Quasi-Newton safeguarding: the BFGS update may be damped or undamped.
        In the SR1 and undamped BFGS cases, the update is skipped if the 
        denominator is near zero.</li>
    <li>Quasi-Newton scaling: an initial scaling of y'y/y's I (Shanno &amp; Phua)
        is used for both BFGS and SR1 to accelerate the updating process.</li>

    </ul>

<li>General support for variable insertion in nested models.  This enables
    (1) optimization under uncertainty, in which design variables are 
    distribution parameters, and (2) second-order probability, in which
    epistemic uncertainties are modeled as intervals on distribution 
    parameters and realizations of these parameters are used within sets of
    aleatory probabilistic analyses.  
    Nested models can update sub-model active/inactive variables using
    any combination of variable insertions and
    augmentations (see variable mappings in 
    <a href="./release/html-ref/MethodCommands.html#MethodIndControl">Method Independent Controls</a>).</li>

<li>Extensions to DAKOTA's library mode.  This mode has now stabilized
    and is in use in several Sandia and external simulation frameworks.
    It supports a "sandwich" implementation in which DAKOTA provides iteration
    services to a simulation framework, and the simulation framework
    provides a direct interface plug-in to DAKOTA.
    The current state is reflected in <a href="./release/html/DakLibrary.html">Interfacing with DAKOTA as a Library</a>.
</li></ul>


<h3>Iterative methods</h3>

<ul>
<li>Optimization:</li>
  <ul>
  <li>New Acro release with updated COLINY methods supporting general 
      nonlinear constraints: APPS, DIRECT, Solis-Wets, Pattern search, 
      and Cobyla.  Updated DAKOTA input specification now captures the
      relevant method options.</li>
  <li>The multiobjective genetic algorithm (MOGA) now returns the "best"
      design to DAKOTA for use in multi-level strategies which require a 
      starting point for subsequent iteration.  This best design is defined
      to be the point in the Pareto set that has minimum distance from the
      "utopia" point (the point of extreme values for each objective
      function).</li>
  </ul>

<li>Parameter estimation:</li>

  <ul>
  <li>Revision of NL2SOL specification mappings to be more consistent
      with other methods.</li>
  </ul>

<li>Uncertainty quantification:</li>
  <ul>
  <li>New capability for variance-based decomposition with the following 
      sampling methods: LHS, DDACE, and FSUDace.  VBD provides global 
      sensitivity indices for each uncertain variable, where a higher 
      sensitivity index means that the uncertainty in that input has a 
      larger effect on the variance of the output.</li>
  <li>Addition of a 2nd-order probability capability as enabled by the
      generic variable insertion capability described above.  An epistemic
      level can sample over uncertain variables, whose realizations are then
      inserted into the distribution parameters for an uncertainty analysis
      at an aleatory level.</li>

  <li>Addition of a subset of the GNU Scientific Library (GSL).  In
      particular, new routines for random number generation (the Tausworthe
      RNG) and various statistical distributions have been added.
      Confidence interval calculations for sample means and sample standard 
      deviations have been added to DAKOTA to take advantage of these
      new capabilities.</li>
  <li>Improved logic in reliability methods for conditionally computing
      response means/std deviations via MVFOSM analysis.  The new
      logic makes use of improved data management in nested models to
      determine when these additional statistics are needed.</li>
  </ul>

<li>Parameter studies/design of experiments:</li>
  <ul>
  <li>New FSUDace library for quasi-Monte Carlo (QMC) and Centroidal 
      Voronoi Tesselation (CVT) designs of experiments.  The quasi-Monte 
      Carlo methods currently available are Hammersley and Halton sequences.
      These new QMC and CVT methods support latinization of samples, 
      calculation of volumetric quality metrics, and variance-based 
      decomposition. This library was developed by Max Gunzburger and John 
      Burkardt at Florida State University.</li>
  <li>DDACE now supports volumetric quality metrics and variance-based 
      decomposition.</li>

  </ul>
</ul>


<h3>Strategies</h3>

<ul>
<li>Surrogate-based optimization (SBO)</li>
  <ul>
  <li>Multifidelity SBO can now use more practical second-order corrections
      based on the numerical and quasi-Newton Hessian capabilities discussed
      above.</li>
  <li>Multifidelity SBO now supports parallelism in both the low and
      high fidelity models, as enabled by the parallelism redesign
      discussed above.</li>

  </ul>

<li>Optimization under uncertainty (OUU)</li>
  <ul>
  <li>The generic variable insertion capability described above enables
      reliability-based design optimization studies where the design
      variables define distribution parameters (means, std deviations) for
      uncertain variables.</li>
  <li>Nested models now compute a request vector for the sub-iterator
      and support gradients/Hessians in their response mappings.
      This enables sub-iterator sensitivities for accelerating nested RBDO.
      Code for RIA/PMA reliability sensitivities has been fleshed out, but
      is not active yet.</li>
  </ul>
</ul>

<h3>Miscellaneous</h3>

<ul>
<li>Bug fixes:</li>
  <ul>
  <li>Extensions to IDR parser to provide more helpful error messages
      in the case of unmatched keywords.</li>
  <li>Improved linear constraint management for mixed variable optimizers and
      additional linear constraint error checking.</li>
  <li>Fixed discrete variable mappings in JEGA.</li>

  <li>Removal of static attributes from ProblemDescDB to correct
      approach of multiple instantiations in library mode.</li>
  <li>Repaired long-standing memory leak in IDR.  Static IDR table pointers
      are now properly deallocated.</li>
  <li>Fixed bug with setup of random, chc, elitist selection in SGOPT GAs.</li>
  </ul>

<li>SQA/Porting:</li>
  <ul>
  <li>Mac OSX port extended to include F90 LHS, MPICH, and BLAS linking
      from g77.</li>

  <li>Initial support for g95 on Linux, OSX, Cygwin.</li>
  <li>Extensions to configure system to support -enable-debugging.</li>
  </ul>
</ul>

<a name="v3.2"></a><h2>Version 3.2 (7/21/04)</h2>

<h3>Iterative methods</h3>

<ul>
<li>Optimization:</li>
  <ul>
  <li>Initial release of JEGA: single-objective and multiobjective genetic
      algorithms supporting mixed variables and general constraints.  This
      capability is in a beta release state (stable and undergoing minor
      refinements).</li>
  <li>Initial release of COLINY: next generation of SGOPT including new 
      APPS and DIRECT algorithms.  This capability is in an alpha release 
      state (still stabilizing).</li>
  <li>Updated versions of SGOPT, PICO, UTILIB, and OPT++.  OPT++ version 2.2 
      adds limited-memory BFGS for quasi-Newton and disaggregated Hessian 
      nonlinear interior-point methods for constrained Gauss-Newton.</li>

  <li>Extensions to optimizers for improved tracking of final solutions in 
      multiobjective optimization problems.  Single-objective optimizers,
      multiobjective optimizers, and least squares solvers are now
      standardized on reporting the full granularity of results (all
      objectives or all least squares terms).  In cases where a 
      single-objective optimizer is used to solve a multiobjective or
      least squares problem (via an externally-applied multiobjective or
      Gauss-Newton reduction), a database search is used to recover the
      full granularity of data.</li>
  </ul>

<li>Parameter estimation:</li>
  <ul>
  <li>New NL2SOL nonlinear least squares solver.  This algorithm provides a
      new capability for problems with large residuals at the solution (i.e.,
      the Gauss-Newton approximation is inaccurate).</li>
  </ul>

<li>Uncertainty quantification:</li>

  <ul>
  <li>New f90 LHS library now distributed with DAKOTA.</li>
  <li>New sampling method statistics: simple correlations (input-input,
      input-output, and output-output) and partial correlations 
      (input-output) for raw and rank data.</li>
  <li>Extended reliability methods supporting the reliability index
      approach (RIA) and the performance measure approach (PMA).  These
      approaches support the use of linearizations in x-space or u-space
      centered at the mean values (x-/u-space AMV variants), iterated
      linearizations in x-space or u-space centered at the most probable
      point estimate (x-/u-space AMV+ variants), and no linearizations
      (FORM variant).  This increases the number of available reliability
      methods from 4 to 11.  The RIA approaches are used for computing
      probability levels that correspond to user-selected response levels
      (forward reliability analysis), and the PMA approaches are used for
      computing the response levels that correspond to user-selected
      probability levels (inverse reliability analysis).  Additional
      features include:</li>
      <ul>
      <li>support of NPSOL sequential quadratic programming or OPT++ 
          nonlinear interior point methods for solving the RIA/PMA most 
          probable point searches.  With the addition of OPT++ support, 
          DAKOTA's reliability methods are now available to users lacking 
          NPSOL licenses.</li>
      <li>analytic objective/constraint gradients are now supported within 
          all NPSOL/OPT++ RIA/PMA evaluation functions.</li>

      <li>warm starts are employed for the linearization point and
          optimizer initial guess for the next AMV+ iteration increment,
          for the next response/probability/reliability level, or for the
          next design variable values.  In some cases, projections are
          used to avoid premature convergence (due to KKT condition
          satisfaction).</li>
      </ul>
  <li>UQ standardization: consistent support for response (z),
      probability (p), and reliability (beta) level specifications in all
      sampling, reliability, and polynomial chaos methods.  Mappings can
      now be z-&gt;p, z-&gt;beta, p-&gt;z, or beta-&gt;z, and the z/p/beta levels may 
      now be mixed within the specifications for each response function.
      Probabilities and reliabilities may now correspond to either cumulative
      or complementary cumulative distribution functions (CDF/CCDF).
      Specific extensions required for standardization include:</li>
      <ul>
      <li>the Mean Value reliability method now supports z/p/beta level
          calculations (previously, only response mean and standard deviation
          estimates were supported).</li>

      <li>MPP search-based reliability methods now support multiple response 
          functions and all z-&gt;p/beta and p/beta-&gt;z mappings (previously, only
          z-&gt;p mappings for a single response function were supported).  The
          new z-&gt;beta mappings are especially important for RBDO since beta
          is more well-behaved and well-scaled than p for use in optimization
          constraints.</li>
      <li>sampling/PCE methods have added support for p-&gt;z, z-&gt;beta, and
          beta-&gt;z for CDF/CCDF probabilities/reliabilities (previously, only
          a single z-&gt;p CCDF mapping per response function was supported).
          The reliability index beta is defined for sampling methods as the
          number of standard deviations separating the mean from the target
          response level.  This gives an alternative metric for resolving
          low probabilities with sampling methods for use in OUU.</li>
      <li>lognormal distribution parameters have now been standardized 
          within sampling and reliability methods.</li>

      </ul>
  </ul>

<li>Parameter studies/design of experiments:</li>
  <ul>
  <li>Addition of a new base class, Dakota::PStudyDACE, which consolidates 
      previously replicated code between parameter study and design of 
      experiments iterators.</li>
  </ul>
</ul>


<h3>Strategies</h3>

<ul>
<li>Surrogate-based optimization (SBO)</li>
  <ul>
  <li>New hybrid penalty/filter logic eliminates erroneous rejection of
      SBO iterates. In particular, the penalty parameter is rescaled so as
      to accept iterates with reductions in constraint violation (penalty
      levels must increase monotonically). This is less restrictive than a
      pure penalty approach but more restrictive than a pure filter approach.
      A pure filter approach will be implemented soon.</li>
  <li>New 2nd-order additive and multiplicative local corrections and new
      2nd-order combined/multipoint corrections have been added.  Improves
      convergence for SBO employing hierarchical surrogates.</li>
  <li>Enhancements to trust region ratio logic and soft/hard convergence
      controls.  The hard convergence check now calculates a projected
      gradient norm in the presence of active bound constraints.</li>

  <li>SBO now overrides the default active set vector within the DACE model.
      This is important when a specification of 1st/2nd derivative information
      is needed to support higher order corrections, but the higher order data
      is not needed for every DACE evaluation in a global surrogate build.</li>
  <li>The Taylor series approximation has been extended to automatically use
      a 2nd-order series whenever analytic Hessians are specified in the
      truth model response specification.</li>
  </ul>

<li>Optimization under uncertainty (OUU)</li>
  <ul>
  <li>Efficiency of trust-region surrogate-based OUU algorithms has been
      enhanced.  In addition, lower-level approximations can now be 
      temporarily bypassed when performing "truth" verifications across 
      multiple surrogate levels.</li>
  <li>New bi-level reliability-based design optimization (RBDO) capabilities.
      These RBDO methods employ warm started reliability analyses (initial
      MPP search initial guess and linearization point).</li>

  <li>Model calibration under uncertainty: OUU formulations involving
      least squares data sets are now supported in addition to
      optimization data sets.</li>
  </ul>
</ul>


<h3>Facilities</h3>

<ul>
<li>Utilities:</li>
  <ul>
  <li>Graphics extensions for better visualization of SBO and reliability
      analysis results.  Reliability analyses now provide CDF/CCDF plots,
      which required 2D plotting extensions to admit general x-y plotting
      (previously restricted to x axis counters), log scale for either/both
      axes (only y axis supported previously), multiple data sets, and plot
      markers with variable anchors.</li>

  <li>DPREPRO now supports assignments involving expressions and precedence
      rules for duplicate assignments (allowing default template file
      assignments to be overridden by parameters file values).</li>
  <li>Restart utility extended to support active/inactive variables,
      response tags, sorting across multiple interfaces, and STL 
      iterator-based list traversals.</li>
  <li>DAKOTA library mode extended with improved support for publishing new
      derived "letter" classes (instantiated outside of DAKOTA) to existing
      "envelope" class hierarchies.</li>
  </ul>

<li>General enhancements:</li>
  <ul>
  <li>New "Dakota" namespace shortens many DAKOTA class names and avoids
      name clashes.</li>

  <li>utilib::PackBuffer replaced with Dakota::MPIPackBuffer.  This
      eliminates the dependence of DAKOTA MPI executables on UTILIB 
      which had been causing problems for AIX builds.</li>
  <li>Redesign and clean up of iterator classes employing function pointer
      designs. Migrated from a design in which many static member attributes
      were maintained solely as copies of nonstatic attributes to a design
      in which these static member copies have been replaced by a single
      static pointer to the active object instance of the iterator class.
      This static pointer provides the needed data access from the static
      member functions used in the function pointer design.</li>
  <li>IDR parser updated to use a factor of two buffer growth strategy.
      This significantly reduces parsing time for large datasets.</li>
  <li>Addition of the ability to specify a vector of finite difference step 
      sizes, one per active continuous variable, for use in the DAKOTA
      finite differencing routine (not supported by vendor routines).</li>
  </ul>

<li>Bug fixes:</li>

  <ul>
  <li>NPSOL modified to prevent moving the initial point to the variable 
      bounds when the initial point is close to the bounds.</li>
  <li>Bug with number of correlation coefficients for one uncertain
      variable corrected within new LHS sampling library.</li>
  <li>Bug fix in OPT++ within the line search backtracking routine
      for the case of zero gradient.</li>
  <li>Optional response tag extraction is now more robust (would fail on
      some platforms).</li>
  <li>Bug fix for parallel initialization of high fidelity interfaces in 
      model-hierarchy SBO.</li>

  <li>Bug fix for uninitialized pointers in the OPT++ interface classes.</li>
  <li>Bug fix in sampling output correlation matrix sizing for OUU.</li>
  <li>Clarification of best data point tracking in parameter studies and design
      of experiments (only optimization and least squares data sets define best
      data metrics, generic response function data sets do not).</li>
  <li>Make recursion bug fixed with addition of checks for missing vendor
      optimizers/packages.</li>
  </ul>

<li>SQA/Porting:</li>

  <ul>
  <li>Automated DAKOTA testing: nightly testing harness is much more thorough
      and includes new unit test and parallel regression test facilities.</li>
  <li>Version of the day (VOTD) distributions are now active for publishing
      the latest code versions to the user base.</li>
  <li>DAKOTA ports and configuration extensions: initial port to Mac OS X 
      completed; configuration support added for GM device of MPI, 64-bit 
      Sun builds, and Sun Cluster Tools MPI (/opt/SUNWhpc); Sun builds 
      updated to Workshop 7.
  </li></ul>
</ul>

<a name="v3.1.2"></a><h2>Version 3.1.2 (7/09/03)</h2>

<h3>Interim release enhancements</h3>

<ul>
<li>Additional refinements to 2D graphics:</li>
  <ul>
  <li>rework of SciPlot numerics to correct problems that were occurring when the
       y axis values become too large and/or when a range is not yet available:</li>
       <ul>

       <li>switched x and y axis numeric labels from a "%f" fixed format 
            to a "%g" output format which automatically switches to exponential 
            format when the numbers become large.  This format also allows for 
            minimal label precision and field width.</li>
       <li>reworked numerics for calculating axis increments and estimating 
            space allocation for the new numeric label formats.</li>
       <li>reduced the maximum number of x axis increments to eliminate some label 
            overlapping and centered the x axis labels on the axis ticks.</li>
       </ul>
  <li>addition of plot window options for axis labels and legends.</li>

  <li>min/max symbols changed to unique shapes (discernable in black/white EPS).</li>
  <li>thumb wheels now support bidirectional windowing (left or right 
       and up or down).</li>
  <li>refinement of initial graphics window sizing for larger numbers of plots.</li>
  <li>elimination of some data skipping in the graphics through suppression
       of data sets which contain only derivative data.</li>
  <li>efficiency enhancements through removal of redundant arrays and
       streamlining of data updates.</li>

  </ul>
<li>New features:</li>
  <ul>
  <li>initial release of <h3>DPREPRO</h3> (<u>D</u>AKOTA 
       <u>pre</u>-<u>pro</u>cessor), a freely-distributable pre-processor 
       for use in interfaces between DAKOTA and various simulation codes.  
       DPREPRO is a Perl script utility that inserts values from the DAKOTA 
       parameters file into a template simulation input file to create new 
       simulation input file instances.  Features include:</li>

       <ul>
       <li>supports both APREPRO and standard formats in the DAKOTA 
            parameters files and auto-detects the format in use.</li>
       <li>extracts variable assignments both from the parameters file and 
            from the template file and supports multiple order-independent 
            matches and replacements per line.</li>
       <li>stores tag/value pairs in a hash for lookup efficiency.</li>
       <li>supports algebraic manipulation using both the intrinsic Perl 
            functions as well as a variety of additional math subroutines.</li>

       </ul>
  </ul>
<li>Miscellaneous:</li>
  <ul>
  <li>fixed a bug with management of the aprepro format for LayeredModels
       which was occurring in hierarchical surrogate-based optimization.</li>
  <li>added a workaround for older pgf90 compilers such as those used on 
       ASCI Red to allow use of the new f90 LHS library within DAKOTA.  To 
       mix C++ with pgf90, an explicit call to pghpf_init() is needed to 
       allow f90 calls to ALLOCATE().</li>
  </ul>

</ul>

<a name="v3.1.1"></a><h2>Version 3.1.1 (5/21/03)</h2>

<h3>Interim release enhancements</h3>

<ul>
<li>Overhaul of 2D graphics:</li>
  <ul>

  <li>subtle bug fixed with passing double* from an STL vector.</li>
  <li>horizontal/vertical wheel operations reworked.</li>
  <li>"Print" and "Markers" buttons activated in "Options"
       window. Markers can now be deactivated, and individual EPS files
       can now be generated from each plot window.</li>
  <li>box sizes, labels, button offsets, and colors adjusted to be more 
       legible, and initial window sizing adjusted to properly capture the 
       number of plots.</li>
  <li>reduced memory usage and improved efficiency.</li>

  </ul>
<li>Completion of parallel scheduling for Pareto-set and Multi-start
strategies:</li>
  <ul>
  <li>fixed parallel bug in random start feature (LHS must be executed 
       only on world rank 0 and its results must be communicated to all
       iterator masters).</li>
  <li>scheduling routines are more modular and dynamic scheduling is 
       now supported within dedicated-master partitions.</li>
  </ul>
<li>Miscellaneous:</li>

  <ul>
  <li>added support for the GM device in MPI.</li>
  <li>streamlined parallel configuration reporting.</li>
  <li>fixed string output formatting problem under Linux.</li>
  </ul>
</ul>

<a name="v3.1"></a><h2>Version 3.1 (4/04/03)</h2>

<h3>New iterative methods</h3>

<ul>
<li>Optimization:</li>
  <ul>

  <li>NPSOL v5.0 update.</li>
  <li>OPT++ method consolidation (from 15 down to 6) and additional
      constraint coverage (added BCFDNewton and GNewton NIPS).</li>
  <li>Initial rSQP++/MOOCHO interface (large-scale optimization).</li>
  <li>Initial COLINY interface for access to APPS and DIRECT optimizers.</li>
  </ul>

<li>Uncertainty quantification:</li>

  <ul>
  <li>New f90 LHS library (not yet publicly available).</li>
    <ul>
    <li>restricted pairing: enforces the desired correlation in the sample
        set, whether inducing a user-specified correlation or minimizing 
        correlation for the default uncorrelated case.</li>
    <li>bounded normal and bounded lognormal distributions</li>
    <li>bin-based and point-based histogram distributions</li>
    <li>additional output: sample histograms and achieved correlation in
        f90 output, coefficient of variation and confidence interval 
        statistics in DAKOTA output.</li>

    <li>consolidation of old/new sampling for LHS/PCE</li>
    </ul>
  <li>MV/AMV/AMV+/FORM reorganization.</li>
  </ul>

<li>Parameter estimation:</li>
  <ul>
  <li>New NLSSOL SQP-based generally-constrained nonlinear least squares.</li>

  <li>New least squares iterator branch to support NLSSOL and OPT++
      GNewton.</li>
  </ul>

<li>Sensitivity/statistical analysis:</li>
  <ul>
  <li>Distribution of DDACE under LGPL.</li>
  <li>Consolidation of function evaluation management routines among 
      design of experiments, UQ sampling, and parameter studies.</li>
  </ul>

</ul>


<h3>New strategies</h3>

<ul>
<li>Surrogate-based optimization</li>
  <ul>
  <li>Enhancements to penalty function consistency.</li>
  <li>Enhancements to trust region management and convergence controls.</li>

  <li>New 1st-order additive correction (thanks to Natalia Alexandrov 
      and Michael Lewis for formulation).  Improves convergence in SBO 
      test cases employing hierarchical surrogates.</li>
  </ul>

<li>Optimization under uncertainty (OUU)</li>
  <ul>
  <li>New trust-region surrogate-based OUU
      (<a href="../papers/MAO2002paper4875.pdf">paper</a>).</li>
  <li>Enhanced data flow in Layered/Nested models.</li>

  </ul>

<li>Concurrent (Pareto set, Multi-start)</li>
  <ul>
  <li>New random starts capability (in place of or in combination with 
      user-specified starts).</li>
  <li>New results/timing summaries.</li>
  <li>Parallel scheduling enhancements.</li>
  </ul>

</ul>


<h3>New DAKOTA facilities</h3>

<ul>
<li>New library mode:</li>
  <ul>
  <li>Library creation (<tt>libdakota.a</tt> et al.) and consolidation in 
      <tt>Dakota/lib</tt>.</li>

  <li>New API for cases when command line arguments are not 
      directly accessible.</li>
  <li>Access to final strategy results.</li>
  <li>Library mode <a href="./release/html-dev/DakLibrary.html">documentation</a>.</li>
  </ul>

<li>New surrogates:</li>
  <ul>

  <li>Linear and cubic polynomials to supplement existing quadratic 
      polynomial.</li>
  </ul>

<li>Enhanced random seed control for multiple sampling sets in DDACE 
    and LHS</li>
  <ul>
  <li>4 seed selections: combinations of fixed vs. sequenced and 
      user-specified vs. system-generated.</li>
  <li>Sequenced seeds result in variable sampling patterns which enhance 
      reliability and accelerate convergence in SBO, OUU, and SBOUU.</li>
  </ul>

<li>New error checking in approximation classes, uncertain variables, PCE, 
    submodel compatibility, and method constraint support.</li>

<li>Efficiency enhancements:</li>
  <ul>
  <li>Dropped pre-ANSI C++ mode which allowed for STL streamlining and
      optimization.</li>
  <li>Additional output control: <tt>debug/verbose/normal/quiet/silent</tt>
      settings, <tt>-i/-o/-e</tt> command line options.</li>

  <li>Evaluation cache and restart deactivation option.</li>
  <li>New MPI intra-communicators to streamline message broadcasts for 
      large parallel runs.</li>
  <li>Streamlined communicator repartitioning in strategies.</li>
  </ul>

<li>New utilities:</li>
  <ul>
  <li>Restart repair by evaluation id.</li>

  <li>3D graphics fonts (PLplot) distributed with executables.</li>
  </ul>

<li>SQA:</li>
  <ul>
  <li>Automated DAKOTA testing: nightly build verifications, new regression 
      test facilities employing test matrices, unit testing under 
      development.</li>
  <li>Nightly library testing for SGOPT, PICO, and UTILIB (OPT++, 
      DDACE to follow).</li>
  <li>Requirements/issue tracking 
      (<a href="http://bugzilla-cs.sandia.gov/bugzilla/">Bugzilla</a>, 
      restricted access), design tracking, release management (automated 
      posting of nightly builds available shortly).</li>

  </ul>
</ul>


<h3>Contributions from the user community</h3>

<ul>
<li>Distributed testing/issue identification:</li>
  <ul>
  <li>SGOPT: problem size limitations removed (identified by Michel 
      Beaulieu), real-valued GA bound constraint bug resolved (identified 
      by Hongman Kim), <tt>multi_step</tt> keyword mismatch resolved
      (identified by Don Flaggs).</li>

  <li>OPT++: bound constraints fixed in PDS (identified by Don Flaggs), 
      final solution returned properly (identified by Michel Beaulieu).</li>
  <li>DOT/CONMIN: registration of automatic method switches for consistency
      in output headers (identified by Don Flaggs).</li>
  <li>SBO fraction of cauchy decrease bug resolved (identified by Myles 
      Baker).</li>
  <li>LINUX graphics configuration generalized (identified by Constantine 
      Orogo).</li>
  <li>IBM POE: replaced short hand checks for <tt>MPI_REQUEST_NULL</tt>
      (identified by Scott Brandon).</li>

  <li>DEC asynchronous evaluations: activated <tt>usleep()</tt> function
      in asynch schedulers as OSF 5.1 is now supporting it (identified by
      Scott Brandon).</li>
  </ul>

<li>Enhancement suggestions:</li>
  <ul>
  <li>Surrogate builds from files (requested by Srinivas Kodiyalam)</li>
  <li>Output of surrogate functional forms (requested by Srinivas 
      Kodiyalam).</li>

  <li>Seed output from SGOPT to allow run recreation (requested by Don 
      Flaggs).</li>
  <li>Response function descriptors (requested by Hongman Kim).</li>
</ul>

<li>Contributed code:</li>
  <ul>
  <li>Cygwin evaluation and initial port (contributed by Hongman Kim).</li>
  <li>Mac OSX evaluation (contributed by Carl Hagelberg).</li>

  <li>Linear polynomial surrogate model (contributed by Myles Baker).</li>
  <li>Direct interface to Phoenix Integration's ModelCenter and merged 
      variable descriptors (contributed by Hongman Kim).</li>
  </ul>
</ul>

<a name="v3.0"></a><h2>Version 3.0 (3/30/02)</h2>


All Version 3.0 beta additions plus:

<h3>General:</h3>

<ul>
<li>Documentation (Users, Reference, and Developers Manuals) updated.</li>
<li>New software quality assurance (SQA) standards instituted.</li>
<li>Vector/list template classes based on STL are now well tested 
and stable.</li>
</ul>

<h3>New strategies:</h3>

<ul>
<li>The surrogate-based optimization (SBO) strategy reduces computational
expense and mitigates nonsmooth response variations through the use of local
(Taylor series), global (quadratic polynomials, neural networks, kriging,
splines), and hierarchical approximations. This strategy is now
provably-convergent through the addition of the "beta" correction approach,
which enforces first order consistency requirements necessary to ensure
convergence to the optimum of the actual, high fidelity model.  This new beta
correction capability is critical for making SBO viable with hierarchical
surrogates (models of varying fidelity).</li>

<li>The augmentation of samples at the root and branch nodes (for identifying
good NLP starting points) is now functional for the parallel branch and bound
strategy.</li>
</ul>

<h3>New iterative methods:</h3>

<ul>
<li>OPT++ has completed the release approval process and the ANSI C++ version
of OPT++ is now available for download with DAKOTA.  The performance of OPT++'s
nonlinear interior point methods is competitive with DOT and NPSOL, and OPT++
therefore provides a more capable freeware alternative for constrained
minimization than CONMIN.</li>

<li>DAKOTA/UQ reorganized to make better use of inheritance, updated from Petra
1.0 to Epetra 2.0, and extended to include the mean value (MV) analytic
reliability method.</li>

<li>DOT/CONMIN wrappers extended to manage linear inequality and equality
constraints.  DOT, NPSOL, OPT++, and CONMIN are now completely plug and play in
terms of linear and nonlinear inequality and equality constraints.</li>
</ul>

<h3>New features:</h3>

<ul>
<li>Tagged output streams for concurrent iterators are now managed with
Cout/Cerr macros and are independent of UTILIB's CommonIO class.</li>

<li>Active set vector control may now be turned off in order to simplify user
simulation interfaces.  This allows the user to neglect the active set vector
and trade some efficiency for simplicity.</li>

<li>LayeredModel correction facilities streamlined to remove the need to back
out previous corrections before applying new ones.  Local and hierarchical
layered models now operate correctly in parallel.</li>
</ul>

<a name="v3.0b"></a><h2>Version 3.0 beta (11/29/01)</h2>

<h3>General:</h3>

<ul>
<li>ANSI C++ compliant: DAKOTA has been updated to the ANSI C++ standard.
Vector/list templates from the commercial Rogue Wave Tools.h++ library have
been replaced with the Standard Template Library (STL) provided as part of the
ANSI C++ standard.</li>

<li>GNU GPL open source release: DAKOTA is now available for download
worldwide.</li>

<li>New documentation: Users, Reference, and Developers Manuals now available
online from 
<a href="../software.html">http://www.cs.sandia.gov/DAKOTA/software.html</a>.
</li>
</ul>

<h3>New strategies:</h3>

<ul>
<li>Optimization under uncertainty (OUU): for robust and reliability-based
design in the presence of parameter uncertainty.  OUU may be executed in simple
nested iteration mode, or it may employ surrogates at the optimization level,
the uncertainty quantification level, or both.</li>

<li>Pareto-set optimization: generates a trade-off surface for multiobjective
optimization problems.  The Pareto-set strategy supports concurrent-iterator
parallelism for large-scale multilevel parallel studies.</li>

<li>Multi-start optimization: runs local optimizations from different starting
points for locating multiple minima. The multi-start strategy supports
concurrent-iterator parallelism for large-scale multilevel parallel
studies.</li>
</ul>

<h3>New iterative methods:</h3>

<ul>
<li>The CONMIN library for gradient-based constrained minimization has been
added to provide an freely available alternative to the commercial DOT and
NPSOL libraries.</li>

<li>Extensions to the DAKOTA/UQ uncertainty quantification library: stochastic
finite element techniques using polynomial chaos expansions have been added in
joint work with Johns Hopkins University.</li>
</ul>


<h3>New features:</h3>

<ul>
<li>New tabular data output capability for postprocessing of DAKOTA results
with external post-processing tools such as Matlab and Tecplot.</li>

<li>Graphics ported to SGI IRIX and PC LINUX platforms and extended for OUU,
multilevel hybrid, Pareto-set, and multi-start strategies.</li>
</ul>
-->

<?php include 'common/footer.html' ?>

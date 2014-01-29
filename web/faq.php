<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - FAQ');
  $page->setNavIdentifier('faq');
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb">FAQ</div>

<ol>
<li><b>Will HPCG replace High Performance Linpack (HPL)?</b>
  <p>
  We do not intend to eliminate HPL.  HPCG will provide
  an alternative ranking of the TOP500 machines. We
  expect that HPCG will take several years to both mature
  and emerge as a widely-visible metric.
  </p>

<li><b>Isn't HPCG just another version of the STREAM benchmark?</b>
  <p>
  If the reference version of HPCG is used for performance
  analysis, the fraction of time spent in the (unoptimized)
  sparse kernels (in particular ComputeSPMV and ComputeSYMGS)
  will be very high, and HPCG performance will be dominated by
  memory system performance.  In this case, for computer systems
  with a good reduction networks, or HPCG runs using few MPI processes,
  the benchmark will
  give rankings that are very similar to STREAM.
  </p>
  <p>
  However, this is true for many benchmarks.  If HPL were executed
  with reference Fortran computational kernels (Basic Linear Algebra
  Subprograms), HPL would also look like a STREAM benchmark.
  </p>
  <p>
  Warnings have been added to HPCG reports to let the benchmarker
  know that performance will be suboptimal when using reference
  kernels.
  </p>

<li><b>Is it permitted to use a custom ordering for the matrix?</b>
  <p>
  Yes, it is permitted to use a custom ordering of the grid points. This is
  facilitated with the function <code>OptimizeProblem()</code> and the
  <code>optimizationData</code> members of various data structures.
  </p>
</li>

<li><b>Why doesn't HPCG include variant X of the CG algorithm?</b>
  <p>
  We are aware of many variants of the CG algorithm and their benefits for
  particular matrices. At the same time, we strive for simplicity of the
  reference implementation and permit only selected optimizations that allow
  the results to remain representative of a wide range of CG variants.
  </p>
</li>

<li><b>Can I change the Gauss-Seidel preconditioner to make it parallel?</b>
  <p>
  It is not permitted to change the preconditioner but it is allowed to change
  the ordering of the matrix to facilitate parallel preconditioning.
  </p>
</li>

<li><b>How is HPCG different from NAS PB CG (NAS Parallel Benchmarks, CG component)?</b>
  <p>
  NAS PB CG uses random sparsity pattern which naturally leads to
  two-dimensional distribution of the matrix for optimality. This results in
  computation and communication patterns that are non-physical. Another
  difference is the lack of preconditioning, does not allow to show the effects
  of local triangular solve. The options for introducing such a preconditioning
  component are limited due to the non-physical sparsity pattern.
  </p>
</li>

<?php include 'common/footer.html' ?>

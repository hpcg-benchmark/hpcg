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
</ol>

<?php include 'common/footer.html' ?>

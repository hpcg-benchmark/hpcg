<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - About');
  $page->setNavIdentifier('about');
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb">About</div>

<h3>About the HPCG Benchmark Project</h3>

<p>The HPCG Benchmark project is an effort to create a more relevant metric for ranking HPC systems than the
High Performance LINPACK (HPL) benchmark, that is currently used by the TOP500 benchmark.  The computational and data
access patterns of HPL are no longer driving computer system designs in directions that are beneficial
to many important scalable applications.  HPCG is designed to exercise computational and data access patterns that more
closely match a broad set of important applications, and to give incentive to computer system designers to invest in
capabilities that will have impact on the collective performance of these applications.
</p>

    <p><b>HPCG is a complete, stand-alone code that measures the performance of basic operations in a unified code:</b>

    <ul>
      <li>Sparse matrix-vector multiplication.</li>
      <li>Sparse triangular solve.</li>
      <li>Vector updates.</li>
      <li>Global dot products.</li>
      <li>Local symmetric Gauss-Seidel smoother.</li>
      <li>Driven by multigrid preconditioned conjugate gradient algorithm that exercises the key kernels on a nested set of coarse grids.</li>
      <li>Reference implementation is written in C++ with MPI and OpenMP support.</li>
  </ul>
  </p>

<h3>HPCG Overview Papers</h3>

<p><a href=doc/HPCG-Benchmark.pdf> Toward a New Metric for Ranking High Performance Computing Systems</a></p>
<p><a href=doc/HPCG-Specification.pdf> HPCG Technical Specification</a></p>


<br/>


<?php include 'common/footer.html' ?>

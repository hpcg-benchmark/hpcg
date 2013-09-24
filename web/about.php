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

<p>The HPCG Benchmark project is an effort to create a more relevant metric for ranking HPC systems 
than the current Top 500 High Performance LINPACK code.</p>

    <p><b>HPCG is a complete, stand-alone code that measures the performance of basic operations in a unified code:</b>
      
    <ul>
      <li>Sparse matrix-vector multiplication.</li>
      <li>Sparse triangular solve.</li>
      <li>Vector updates.</li>
      <li>Global dot products.</li>
      <li>Driven by an additive Schwarz, symmetric Gauss-Seidel preconditioned conjugate gradient algorithm.</li>
      <li>Reference implementation is written in C++ with MPI and OpenMP support.</li>
  </ul>
  </p>
      
<h3>HPCG Overview Papers</h3>

<p><a href=HPCG-Benchmark.pdf> Toward a New Metric for Ranking High Performance Computing Systems</a></p>
<p><a href=HPCG-Specification.pdf> HPCG Technical Specification</a></p>

      
<br/>


<?php include 'common/footer.html' ?>

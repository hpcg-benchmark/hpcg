<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - Release Notes');
  $page->setNavIdentifier('download');

  $y = <<< END
    h2 { padding-top: 2em; }
END;
  $page->setInlineStyles($y);

?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="download.php">Download</a> - Release Notes</div>

<h3> HPCG Benchmark Release 2.4</h3>

<p>Release 2.4 fixes a flop count error introduced in Release 2.2 and not completely resolved in Release 2.3.</p>

<h3> HPCG Benchmark Release 2.3</h3>

<p>Release 2.3 fixes a flop count error introduced in Release 2.2.</p>

<h3> HPCG Benchmark Release 2.2</h3>

<p>Release 2.2 reduces the penalty for optimization overhead costs by a factor of 10.  It also contain several minor bugfixes.</p>

<h3> HPCG Benchmark Release 2.1</h3>

<p>HPCG 2.0/2.1 provides the first implementation of a multigrid preconditioner for the conjugate gradient method.
<ul>
<li> HPCG 2.0/2.1replaces the additive Schwarz preconditioner with a synthetic multigrid approach. </li>
<li> The number of coarse grid levels is parametrized but fixed for production benchmark runs (presently set to 3 levels of coarsening).</li>
<li> We use injection as the grid transfer operator and symmetric Gauss-Seidel as the pre and post smoother.</li>
<li> All kernels from HPCG 1.1 remain important and no new kernels were added.
<li> The biggest impact of the multigrid preconditioner is that all computation and communication kernels will execute on
     a nested sequence of coarse grids where each grid level is 8 times smaller than the previous level.</li>
<li> Every attempt was made to assure that the kernels are not unnecessarily different between HPCG 1.1 and HPCG 2.0.</li>
<li> Even so, some changes were necessary to supported the nested hierarchy.</li>
<li> We did not implement a true multigrid preconditioner.  In particular, we do not have a "bottom" solver for the coarsest grid.</li>
<li> We also added a Vector struct to facility easier implementation of kernels on discrete devices such as GPUs.</li>
</p>

<h3> HPCG Benchmark Release 1.1</h3>

<p>This is the first update release of HPCG. See HISTORY file for more detailed list of changes.</p>

<h3> HPCG Benchmark Release 1.0</h3>

<p>This is the first official release of HPCG for general public access.</p>


<?php include 'common/footer.html' ?>

<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('Mantevo - Packages');
  $page->setNavIdentifier('packages');
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb">Packages</div>

    <ul>
	<li><b>Miniapplications:</b></li>
    <ul>
     <li>*<b>CloverLeaf:</b>
              <a href="http://warwick-pcav.github.com/CloverLeaf">CloverLeaf</a>
              is a miniapp that solves the compressible Euler equations on a
              Cartesian grid, using an explicit, second-order accurate
              method.</li>
     <li>*<b>CoMD:</b> A simple proxy for the computations in a typical
              molecular dynamics application. The reference implementation
              mimics that of SPaSM. In addition, we provide an OpenCL
              implementation which allows testing on multicore and GPU
              architectures, with both array-of-structures and
              structure-of-arrays data layouts. More information can be found
              at <a href="https://github.com/exmatex/CoMD">https://github.com/exmatex/CoMD</a>.</li>
     <li>*<b>MiniFE:</b> MiniFE is an proxy application for unstructured implicit finite element codes.  
             It is similar to HPCCG and pHPCCG but provides a much more complete vertical covering of 
             the steps in this class of applications.  MiniFE also provides support for computation on multicore
             nodes, including pthreads and Intel Threading Building Blocks (TBB) for homogeneous multicore and 
             CUDA for GPUs.  Like HPCCG and pHPCCG, MiniFE is intended to be the "best approximation to an 
             unstructured implicit finite element or finite volume application, but in 8000 lines or fewer."</li>
      <li>*<b>HPCCG:</b> Intended to be the "best approximation to an unstructured implicit finite 
	     element or finite volume application in 800 lines or fewer."</li>
      <li><b>pHPCCG:</b> A parametrized version of HPCCG that supports use of different scalar and integer data types, 
	     as well as different sparse matrix data structures.</li>
      <li>*<b>MiniMD:</b> A simple proxy for the force computations in a typical molecular dynamics applications.  The
              algorithms and implementation used closely mimics these same operations as performed in LAMMPS.</li>
      <li>*<b>MiniGhost:</b> A Finite Difference proxy application which 
implements a difference stencil across a homogenous three dimensional
domain.</li>
      <li> <b>phdMesh:</b> Parallel heterogeneous dynamic mesh application.  Exhibit the performance characteristics 
	     of the contact search operations in an explicit finite element application.</li>
      <li>*<b>MiniXyce:</b> A portable proxy of some of the key capabilities in the electrical modeling 
              Xyce.</li>
    </ul>
      <li><b>Minidrivers:</b></li>
    <ul>
      <li><b>Beam:</b> Wraps the Trilinos packages for forming element stiffness matrices (the Intrepid package), 
	     assembling them (the FEI package) and solving the resulting  linear system (using AztecOO, ML, Ifpack and 
	     Epetra packages).</li>
      <li>*<b>Epetra Kernels Benchmark:</b> Driver that executes key performance-impacting Epetra kernels for 
	sparse matrix-vector, sparse matrix-multivector and dense kernels.</li>
    </ul>
    <li><b>Motif frameworks:</b> 
      <ul>
	<li><b>Prolego:</b> Driven by XML data sets that choose code fragments and weightings, Prolego can be 
	  easily configured to mimic the performance profile of a large 
	  variety of implicit and explicit unstructure finite element/volume applications.
      </ul>
    </ul>

<p>* Denotes a miniapp/minidriver that is included in the Mantevo Suite 1.0
release.</p>
<?php include 'common/footer.html' ?>

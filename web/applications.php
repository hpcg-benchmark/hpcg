<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - Applications');
  $page->setNavIdentifier('about');

  $x = <<< END
    img.app { float: right; padding: 10px 10px 20px 20px; }
END;
  $page->setInlineStyles($x); 
  
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="about.php">About</a> - Applications</div>

<h3>Applications</h3>

    <p><b>Current activities:</b>

HPCG miniapps and minidrivers target a variety of application domains.  Presently we address aspects of the following domains.
	
<ul>
<li><b>Implicit unstructured partial differential equations:</b> 
       Although this is a very broad area, a number of HPCG packages address it fairly well.
       This is because many of these applications have a highly localized, operation-intensive phase for constructing a sparse linear
       system, followed by solution of the system via an iterative sparse solver.  This is the most mature area for HPCG since it
       is one of the most important areas for Sandia, where HPCG started. The following miniapps and minidrivers address this domain:</li>
    <ul>
      <li><b>MiniFE:</b> (Miniapp) Intended to be the best approximation to an unstructured implicit finite that includes 
                         all important computational phases.</li>
      <li><b>HPCCG:</b> (Miniapp) Similar to MiniFE, but generates a synthetic linear system.  The focus is entirely on the 
                        sparse iterative solver. </li>
      <li><b>Beam:</b> (Minidriver) Wraps the Trilinos packages for forming element stiffness matrices (the Intrepid package), 
	assembling them (the FEI package) and solving the resulting  linear system (using AztecOO, ML, Ifpack and 
	Epetra packages.</li>
      <li><b>Epetra Kernels Benchmark:</b> (Minidriver) Driver that executes key performance-impacting Epetra kernels for 
	sparse matrix-vector, sparse matrix-multivector and dense kernels.</li>
    </ul>

<li><b>Explicit unstructured partial differential equations:</b> 
       Again this is a very broad area, and very important.  This domain is typified by indirect
       addressing, vector operations and contact detection.  HPCG has one miniapp in this area, and a few others under development.</li>
    <ul>
      <li> <b>phdMesh:</b> Parallel heterogeneous dynamic mesh application.  Exhibits the performance characteristics 
	of the contact search operations in an explicit finite element application.</li>
    </ul>

<li><b>Explicit structured partial differential equations:</b> 
       Again this is a very broad area, and very important.  This domain is typified by highly structured stencil operations, especially
       communication for "halo exchange" operations.</li>
    <ul>
      <li> <b>MiniGhost:</b> Executes the halo exchange pattern of important structured and block-structured explicit applications.</li>
    </ul>

<li><b>Molecular Dynamics:</b></li>
    <ul>
      <li> <b>MiniMD:</b> A light-weight molecular dynamics application containing the performance impacting code 
	              from <a href="http://lammps.sandia.gov">LAMMPS</a>.</li>
      <li> <b>CoMD:</b> An extensible molecular dynamics proxy applications
suite featuring the Lennard-Jones potential and the Embedded Atom Method potential.</li>
    </ul>

<li><b>Hydrodynamics:</b> These codes investigate the behavior and responses of
materials when applied with varying levels of stress. It is common for
hydrocodes to be constructed using one of two formulations - Lagrangian, in
which a mesh is constructed and evolved through time, or Eulerian, where a
fixed spatial region is analyzed.</li>
    <ul>
      <li> <b>Cloverleaf:</b> Uses a two-dimensional Eulerian formulation.</li>
    </ul>

<li><b>Circuit Simluation:</b> This area is very important and quite different than PDE-based domains since the 
                               problem is inherently discrete.</li>
    <ul>
      <li> <b>MiniXyce:</b> Named after Sandia's parallel circuit simulation code "<a href="http://xyce.sandia.gov">Xyce</a>.</li>
    </ul>
</ul>
  </p><br/><br/>

<?php include 'common/footer.html' ?>

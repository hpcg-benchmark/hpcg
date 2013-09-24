<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('Mantevo - Research');
  $page->setNavIdentifier('about');
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="about.php">About</a> - Research</div>

<p>Research results will be posted here as available.</p>

<!--
<p>Research programs have focused on several areas:</p>

<uL>
<li>Parallel processing.  Selected publications include:<br/><br/>

     <ul> 
     <li>
     <a href="./papers/StrOpt_JPaper.pdf">
     "Multilevel Parallel Optimization Using Massively Parallel Structural 
      Dynamics"</a>.

     <li> <a href="./papers/MDO2000_paper.pdf">
     "Multilevel Parallelism for Optimization on MP Computers: Theory and
      Experiment"</a>.
     </ul><br/>

<li>Surrogate-based optimization.<br/><br/>

     <ul>
     <li> SBO with <a HREF="./papers/MDO2000_sao_paper.pdf">data fits</a>.
     <li> SBO with <a HREF="./papers/SBO_Corr_MAO_paper.pdf">multifidelity models</a>.
     <li> SBO with reduced-order modeling (ROM).
     </ul><br/>

<li>Uncertainty quantification. Selected publications include:<br/><br/>

     <ul>
     <li> <a href="./papers/SIE-KM-005.pdf"> 
     "Investigation of Reliability Method Formulations in DAKOTA/UQ"</a>.

     <li> <a href="./papers/SDM2001_uq_paper.pdf">
     "A Toolkit For Uncertainty Quantification In Large Computational
     Engineering Models"</a>.
     </ul><br/>

<li> Optimization under uncertainty.<br/><br/>

     <ul>
     <li> <a href="./papers/MAO2002paper4875.pdf">Surrogate-based</a>.
     <li> <a href="./papers/SIE-KM-005.pdf">Reliability-based</a>.
     </ul> 

</ul>
-->

<?php include 'common/footer.html' ?>

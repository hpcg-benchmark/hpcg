<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - Software Design');
  $page->setNavIdentifier('about');
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="about.php">About</a> - Software Design</div>

<p> HPCG is a self-contained C++ program with MPI and OpenMP support.</p>

<br/>


<?php include 'common/footer.html' ?>

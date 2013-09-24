<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - Publications');
  $page->setNavIdentifier('resources');
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="resources.php">Resources</a> - Publications</div>

<ul class="docs">
<li><a href="HPCG-Benchmark.pdf"> Toward a New Metric for Ranking High Performance Computing Systems (.pdf)</a></li>
<li><a href="HPCG-Specification.pdf"> HPCG Technical Specification (.pdf)</a></li>
</ul>

<?php include 'common/footer.html' ?>

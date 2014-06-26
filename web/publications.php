<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - Publications/Presentations');
  $page->setNavIdentifier('resources');
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="resources.php">Resources</a> - Publications/Presentations</div>

<ul class="docs">
<li><a href="doc/HPCG-Benchmark.pdf"> Toward a New Metric for Ranking High Performance Computing Systems (.pdf)</a></li>
<li><a href="doc/HPCG-Specification.pdf"> HPCG Technical Specification (.pdf)</a></li>
<li><a href="http://www.netlib.org/utk/people/JackDongarra/SLIDES/sc13-top500-hpcg-FINAL.pdf"> HPCG Overview Talk from SC'13 TOP 500 BOF.(.pdf)</a></li>
<li><a href="http://www.netlib.org/utk/people/JackDongarra/SLIDES/hpcg-isc-0614.pdf"> HPCG: One Year Later, from ISC'14.(.pdf)</a></li>
</ul>

<?php include 'common/footer.html' ?>

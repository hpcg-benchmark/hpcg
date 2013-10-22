<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - Packages');
  $page->setNavIdentifier('packages');
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb">Packages</div>

    <ul>
	<li><b>Versions of HPCG:</b></li>
    <ul>
     <li><b>Reference:</b> <a href="downloads/hpcg-0.4.tar.gz">HPCG 0.4 Reference code (October 21, 2013)</a>
     <li><b>Reference:</b> <a href="downloads/hpcg-0.3.tar.gz">HPCG 0.3 Reference code (September 25, 2013)</a>
      </ul>
    </ul>

<?php include 'common/footer.html' ?>

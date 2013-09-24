<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - License');
  $page->setNavIdentifier('download');
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="download.php">Download</a> - License</div>


<p>HPCG proxy applications are open source software.  Some HPCG proxy
applications are available for download under a 
<a href="http://opensource.org/licenses/BSD-3-Clause">New BSD License</a>.</p>

<?php include 'common/footer.html' ?>

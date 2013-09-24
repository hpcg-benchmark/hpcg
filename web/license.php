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
<a href="http://www.gnu.org/copyleft/lgpl.html">GNU Lesser General Public License (LGPL)</a>,
others are available under a <a href="https://github.com/exmatex/CoMD/blob/master/copyright.txt">BSD License</a>. The license for each proxy app is specified
 in the applicable tarball.</p>

<?php include 'common/footer.html' ?>

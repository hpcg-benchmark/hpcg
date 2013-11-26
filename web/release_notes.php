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

<h3> HPCG Benchmark Release 1.1</h3>

<p>This is the first update release of HPCG. See HISTORY file for more detailed list of changes.</p>

<h3> HPCG Benchmark Release 1.0</h3>

<p>This is the first official release of HPCG for general public access.</p>


<?php include 'common/footer.html' ?>

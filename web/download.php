<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - Download');
  $page->setNavIdentifier('download');
 
  $y = <<< END
    th { text-align: left; padding-right: 1em; vertical-align: top; 
      padding-bottom: 1em; }
    td { vertical-align: top; padding-bottom: 1em; }
END;
  $page->setInlineStyles($y);
  
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb">Download</div>

<h3>HPCG Benchmark Release Version 0.5 (Alpha) </h3>

<p>The current HPCG Suite Release version is 0.5. </p>

<p>Previous releases can be found <a href="packages.php">here</a>.</p>

  HPCG: <a href="downloads/hpcg-0.5.tar.gz">Reference Version 0.5 (25-Oct-2013)</a>

<?php include 'common/footer.html' ?>

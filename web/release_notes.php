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

<h3> HPCG Benchmark Release 0.3</h3>

<p>This is the alpha release of HPCG for selected trial access.</p>


<?php include 'common/footer.html' ?>

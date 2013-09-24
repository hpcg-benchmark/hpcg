<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - News');
  $page->setNavIdentifier('home');
 
  $y = <<< END
    div#contentMain li { padding-bottom: 1em; }
END;
  $page->setInlineStyles($y);
  
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="default.php">Home</a> - News</div>

<strong>Monday, November 18th, 2013</strong> - HPCG Benchmark Release 1.0, the
first release of the HPCG Suite, is now <a href="download.php">available for download</a>.  See the <a href="release_notes.php">release notes</a> for more information.<br/><br/>

<br/>

<?php include 'common/footer.html' ?>

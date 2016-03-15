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

<strong>Thursday, June 26th, 2014</strong> - First list of HPCG ranking results <a href=results.php">released</a>.
<br/><br/>

<strong>Monday, June 3nd, 2014</strong> - HPCG Benchmark Release 2.4 fixes a floating point operation count issue that persisted from Release 2.2.
 Release 2.3 is <a href="download.php">available for download</a>.  See the <a href="release_notes.php">release notes</a> for more information.<br/><br/>

<strong>Monday, June 2nd, 2014</strong> - HPCG Benchmark Release 2.3 fixes a floating point operation count issue in Release 2.2.
 Release 2.3 is <a href="download.php">available for download</a>.  See the <a href="release_notes.php">release notes</a> for more information.<br/><br/>

<strong>Wednesday, May 27th, 2014</strong> - HPCG Benchmark Release 2.2, with
reduced penalty for optimization overhead cost, is now <a href="download.php">available for download</a>.  See the <a href="release_notes.php">release notes</a> for more information.<br/><br/>

<strong>Friday, January 31st, 2014</strong> - HPCG Benchmark Release 2.1, the
first Multigrid preconditioned version of HPCG , is now <a href="download.php">available for download</a>.  See the <a href="release_notes.php">release notes</a> for more information.<br/><br/>

<strong>Tuesday, November 26th, 2013</strong> - HPCG Benchmark Release 1.1, the
first update release of the HPCG Suite, is now <a href="download.php">available for download</a>.  See the <a href="release_notes.php">release notes</a> for more information.<br/><br/>

<strong>Monday, November 19th, 2013</strong> - HPCG Benchmark Release 1.0, the
first release of the HPCG Suite, is now <a href="download.php">available for download</a>.  See the <a href="release_notes.php">release notes</a> for more information.<br/><br/>

<br/>

<?php include 'common/footer.html' ?>

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

<h3>HPCG Benchmark Release Version 1.1</h3>

<p>The current HPCG Suite Release version is 1.1. </p>


  HPCG: <a href="downloads/hpcg-1.1.tar.gz">Reference Version 1.0 (19-Nov-2013)</a>

    <ul>
        <li><b>Previous versions of HPCG:</b></li>
    <ul>
     <li><b>Reference:</b> <a href="downloads/hpcg-1.0.tar.gz">HPCG 1.0 Reference code (November 19, 2013)</a>
     <li><b>Reference:</b> <a href="downloads/hpcg-0.5.tar.gz">HPCG 0.5 Reference code (October 25, 2013)</a>
     <li><b>Reference:</b> <a href="downloads/hpcg-0.4.tar.gz">HPCG 0.4 Reference code (October 21, 2013)</a>
     <li><b>Reference:</b> <a href="downloads/hpcg-0.3.tar.gz">HPCG 0.3 Reference code (September 25, 2013)</a>
      </ul>
    </ul>

<?php include 'common/footer.html' ?>

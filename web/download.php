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

<h3>HPCG Benchmark Release Version 2.4</h3>

<p>The current HPCG Suite Release version is 2.4. </p>


  HPCG: <a href="downloads/hpcg-2.4.tar.gz">Reference Version 2.4 (3-Jun-2014)</a>

    <ul>
        <li><b>Previous versions of HPCG:</b></li>
    <ul>
     <li><b>Reference:</b> <a href="downloads/hpcg-2.3.tar.gz">HPCG 2.3 Reference code (June 2, 2014)</a>
     <li><b>Reference:</b> <a href="downloads/hpcg-2.2.tar.gz">HPCG 2.2 Reference code (May 27, 2014)</a>
     <li><b>Reference:</b> <a href="downloads/hpcg-2.1.tar.gz">HPCG 2.1 Reference code (January 31, 2014)</a>
     <li><b>Reference:</b> <a href="downloads/hpcg-2.0.tar.gz">HPCG 2.0 Reference code (January 29, 2013)</a>
     <li><b>Reference:</b> <a href="downloads/hpcg-1.1.tar.gz">HPCG 1.1 Reference code (November 26, 2013)</a>
     <li><b>Reference:</b> <a href="downloads/hpcg-1.0.tar.gz">HPCG 1.0 Reference code (November 19, 2013)</a>
     <li><b>Reference:</b> <a href="downloads/hpcg-0.5.tar.gz">HPCG 0.5 Reference code (October 25, 2013)</a>
     <li><b>Reference:</b> <a href="downloads/hpcg-0.4.tar.gz">HPCG 0.4 Reference code (October 21, 2013)</a>
     <li><b>Reference:</b> <a href="downloads/hpcg-0.3.tar.gz">HPCG 0.3 Reference code (September 25, 2013)</a>
      </ul>
    </ul>

<?php include 'common/footer.html' ?>

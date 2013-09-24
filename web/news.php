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

<strong>Thursday, December 13th, 2012</strong> - HPCG Suite Release 1.0, the
first release of the HPCG Suite, is now <a href="download.php">available for download</a>.  See the <a href="release_notes.php">release notes</a> for more information.<br/><br/>

<!--

<ul>

<li><a href="http://www.cs.sandia.gov/DAKOTA/papers/Dept1533_FactSheet_2006.pdf">2006 Risk-Informed Design Flyer</a></li>

<li><a href="http://www.sandia.gov/media/NewsRel/NR2002/DAKOTA.htm">April 2002 Press Release</a></li>

<li><a href="http://www.sandia.gov/LabNews/LN11-16-01/key11-16-01_stories.html#dakota">November 2001 Lab News article</a></li>

<li><a href="http://www.cs.sandia.gov/DAKOTA/papers/SC2000Final.pdf">Supercomputing 2000 flyer</a></li>

</ul>
-->
<br/>

<?php include 'common/footer.html' ?>

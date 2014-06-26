<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - Events');
  $page->setNavIdentifier('events');
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb">Events</div>


<p>Past HPCG-related events.</p>
<ul>
<li><a href="http://www.netlib.org/utk/people/JackDongarra/SLIDES/sc13-top500-hpcg-FINAL.pdf"> HPCG Overview Talk from SC'13 TOP 500 BOF.(.pdf)</a></li>
<li> <a href="https://www.orau.gov/hpcg2014/default.htm">DOE ASCR HPCG Workshop</a>, March 25, 2014, Washington, DC, USA.</li>
<li> <a href="http://www.isc-events.com/isc14_ap/presentationdetails.htm?t=presentation&o=22&a=select&ra=sessiondetails"> HPCG Presentation at ISC'14</a> ISC'14, June 26, 2014 , Leipzig, Germany.</li>
</ul>

<p>Upcoming HPCG-related events.</p>
<ul>
<li> <a href="http://supercomputing.org"> HPCG BOF at SC'14 (Proposed)</a> SC'14, November, 2014 , New Orleans, LA.</li>
</ul>


<?php include 'common/footer.html' ?>

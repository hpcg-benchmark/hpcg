<?php 
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
      $page->setPageTitle('Mantevo - Home');
      $page->setNavIdentifier('home');
      $page->setPageIdentifier('home');
      ?>
      
<?php include 'common/header.html' ?>

<!--Stat tracking-->
<script type="text/javascript">

  var _gaq = _gaq || [];
  _gaq.push(['_setAccount', 'UA-36185816-1']);
  _gaq.push(['_trackPageview']);

  (function() {
    var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
    ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
    var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s);
  })();

</script>
      
<!--
<div class="releaseBoxContainer">
<div class="releaseBox">
  <p>The current release update is: <strong>1.0</strong></p>

  <p>Released: <strong>January 1, 2008</strong></p>

  <p><a href="download.php">Download Mantevo 1.0 now</a>.</p>
</div>
</div>
-->

<h2>Welcome to the Mantevo Project Home Page</h2>

    <p>Mantevo is a multi-faceted application performance project.  It provides application proxies of several types and sizes:

    <ul>
      <li><b>Miniapplications:</b> Small, self-contained programs that embody essential performance characteristics of key applications.</li>
      <li><b>Minidrivers:</b> Small programs that act as drivers of performance-impacting Trilinos packages.</li>
      <li><b>Application proxies:</b> Parametrizable applications that can be calibrated to mimic the performance of a large scale application, then used as a proxy 
	for the large scale application.</li>
    </ul>
  </p>

<h2>Mantevo Suite Release 1.0</h2>

<p>Mantevo Suite Release 1.0, the first release of the Mantevo Suite, is
now <a href="download.php">available for download</a>.  See the
 <a href="release_notes.php">release notes</a> for more information.</p>
    
<h2>Mantevo Overview Paper</h2>

<p><a href=MantevoOverview.pdf> Improving Performance via Mini-applications (Mantevo Overview) </a></p>

<h2>Mantevo Virtual Machine</h2>

<p>A <a href=https://www.virtualbox.org>VirtualBox</a> virtual machine preloaded with the proxy apps featured at the SC12 Proxy App Tutorial is <a href=./VirtualMachine.php>available for download</a>.  Using the virtual 
machine is perhaps the simplest way to try some of the Mantevo proxy apps for 
the first time.  VirtualBox runs on Linux, Mac, or Windows.</p>

<?php include 'common/footer.html' ?>

<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('Mantevo - Download');
  $page->setNavIdentifier('download');
 
  $y = <<< END
    th { text-align: left; padding-right: 1em; vertical-align: top; 
      padding-bottom: 1em; }
    td { vertical-align: top; padding-bottom: 1em; }
END;
  $page->setInlineStyles($y);
  
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

<div class="breadcrumb">Download</div>

<h3>Mantevo Suite Release Version 1.0.2</h3>

<p>The current Mantevo Suite Release version is 1.0.2.  Each proxy application
included in the Suite is distributed separately.  Mantevo Suite Release 1.0.2
is comprised of seven miniapplications, and one minidriver. This release
contains the same version of many of the proxy applications as the 1.0.1 Suite
Release. Whenever a new version of a proxy application is released, 
the Suite Release version is incremented.</p>

<p>Previous releases can be found <a href="download_previous.php">here</a>.</p>

<ul>
 <li> <b>Miniapps</b>:
 <ul>
  <li> <a href="http://warwick-pcav.github.com/CloverLeaf">CloverLeaf</a>:
       <a href="downloads/CloverLeaf-1.0.html">Version 1.0</a>,
       <a href="downloads/CloverLeaf_ref-1.0.html">Reference Version 1.0</a>
  </li>
  <li> <b>**</b><a href="https://github.com/exmatex/CoMD">CoMD</a>:
       <a href="downloads/CoMD_ref-1.1.html">Reference Version 1.1</a>
  </li>
  <li> HPCCG: <a href="downloads/HPCCG-1.0.html">Reference Version 1.0</a>
  </li>
  <li> MiniFE: <a href="downloads/miniFE_1.5.html">Version 1.5</a>, 
       <a href="downloads/miniFE_ref_1.5.html">Reference Version 1.5</a>
  </li>
  <li> MiniGhost: <a href="downloads/miniGhost_1.0.html">Version 1.0</a>,
       <a href="downloads/miniGhost_ref_1.0.html">Reference Version 1.0</a>
  </li>
  <li> MiniMD: <a href="downloads/miniMD_1.0.html">Version 1.0</a>,
       <a href="downloads/miniMD_ref_1.0.html">Reference Version 1.0</a>
  </li>
  <li> MiniXyce: <a href="downloads/miniXyce_1.0.html">Reference Version 1.0</a>
  </li>
 </ul>
 </li>
 <li> <b>Minidrivers</b>:
 <ul>
  <li> EpetraBenchmarkTest:
       <a href="downloads/EpetraBenchmarkTest-1.0.html">Version 1.0</a>
  </li>
 </ul>
 </li>
</ul>

<p><b>**</b> New version for Suite Release 1.0.2.

<h3>Mantevo Tools</h3>

<ul>
<li><a href="https://github.com/dwbarne/PYLOTDB">Co-Pylot</a>: Post-processing tool that accepts Mantevo YAML output files and stores each as a record in a MySQL database and can display the results in graphs and charts.</li>
</ul>
<br/>
<h2>Open Source Software</h2>

<p>Mantevo proxy applications are open source software.  Some Mantevo proxy
applications are available for download under a
<a href="http://www.gnu.org/copyleft/lgpl.html">GNU Lesser General Public License (LGPL)</a>,
others are available under a <a href="https://github.com/exmatex/CoMD/blob/master/copyright.txt">BSD License</a>. The license for each proxy app is specified
 in the applicable tarball.  Release announcements will be sent
to the Mantevo-Announce@software.sandia.gov mail list.</p>

<!--
<ul>
<li> To initiate the download, first fill out the short
<a href="http://www.cs.sandia.gov/web1400/1400_download.html">Registration 
Form</a> (Note: this information is only used to provide usage statistics to
our program sponsors).

<li> Source code tar file distributions are provided, as well as binary
distributions for selected platforms.  The most recent stable release is
Version 4.0 (5/12/06).  Version of the day (VOTD) releases are also available
for access to the latest developments.

<li><a href="./release_notes.html">Release notes</a> are
available for major, interim, and developmental (VOTD) releases.

<li>Subscriptions to the <a href="http://software.sandia.gov/mailman/listinfo/dakota-announce">dakota-announce</a> and 
<a href="http://software.sandia.gov/mailman/listinfo/dakota-users">dakota-users</a> email lists are available.

<li>If you experience difficulties, please refer to the 
<a href="faq.html">FAQ</a> and to the 
<a href="http://software.sandia.gov/pipermail/dakota-users/">archives</a> of 
dakota-users.  If neither resolves your problem, you may seek support from from
the <a href="mailto: dakota-users@software.sandia.gov">dakota-users</a> forum 
or directly from 
<a href="mailto:dakota-developers@development.sandia.gov">dakota-developers</a>.

</ul><br/>

<h2>Supported Platforms</h2>

DAKOTA runs on most Unix platforms and ports are actively maintained with
nightly build verifications for PC Red Hat Linux, Sun Solaris, IBM AIX, SGI
IRIX, and DEC OSF.  It also runs under Windows (via Cygwin) and Mac OSX, and
has run under HP HPUX in the past, although these ports are not rigorously
maintained at this time.<p>
-->

<?php include 'common/footer.html' ?>

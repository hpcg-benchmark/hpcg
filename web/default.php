<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
      $page->setPageTitle('HPCG - Home');
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
  <p>The current release update is: <strong>2.4</strong></p>

  <p>Released: <strong>June 3, 2014</strong></p>

</div>
</div>
-->

<h2>Welcome to the HPCG Project Home Page</h2>

<?php include 'common/footer.html' ?>

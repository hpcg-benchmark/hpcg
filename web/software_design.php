<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - Software Design');
  $page->setNavIdentifier('about');
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="about.php">About</a> - Software Design</div>

<p> HPCG packages are designed to be used in many different system environments, but are especially meant to be usable
    in new system environments such as simulators and new architecture systems.  As a result, HPCG packages:
<ul>
<li> Are self-contained and rely on simple, manual build processes.</li> 
<li> Use a stable core subset of the C and C++ language environments.</li>
<li> Are small so that they can be easily understood, and even rewritten in new languages.</li>
</ul>


</p>

<br/>


<?php include 'common/footer.html' ?>

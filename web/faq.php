<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - FAQ');
  $page->setNavIdentifier('resources');

  $x = <<< END
END;
  $page->setInlineStyles($x); 
  
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="resources.php">Resources</a> - FAQ</div>

<p>None at this point.</p>

<?php include 'common/footer.html' ?>

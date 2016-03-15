<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - Applications');
  $page->setNavIdentifier('about');

  $x = <<< END
    img.app { float: right; padding: 10px 10px 20px 20px; }
END;
  $page->setInlineStyles($x);

?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="about.php">About</a> - Applications</div>

<h3>Applications</h3>

    <p><b>Current activities:</b>

We have released HPCG Version 2.4 for general availability.

  </p><br/><br/>

<?php include 'common/footer.html' ?>

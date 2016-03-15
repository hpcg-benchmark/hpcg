<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - Documentation');
  $page->setNavIdentifier('resources');

  $x = <<< END
    table.docs td { padding: .5em 2em .5em 0; border-bottom: 1px solid #999; }
    ul.docs li { padding-bottom: 1em; }
END;
  $page->setInlineStyles($x);

?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="resources.php">Resources</a> - Documentation</div>

<p>Reference Documentation for HPCG source files is <a href="html/index.html">here.</a></p>

<?php include 'common/footer.html' ?>

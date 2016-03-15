<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - Developer Tools');
  $page->setNavIdentifier('resources');

  $x = <<< END
    table.lists th { text-align: left; padding: .5em 2em .5em 0;
      border-bottom: 1px solid #999; }
    table.lists td { padding: .5em 2em .5em 0; border-bottom: 1px solid #999; }
END;
  $page->setInlineStyles($x);

?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="resources.php">Resources</a> - Developer Tools</div>

<p>Please contact the project leader for access to HPCG developer tools.</p>
<p>Once you have an account on software.sandia.gov and are a member of the Unix groups HPCGDevelopers and HPCGUsers, you can obtain a working copy of the archive using the following command (note this is a single command, even if it appears to be two):</p>
<p>svn checkout svn+ssh://username@software.sandia.gov/space/sandiasvn/private/hpcg/trunk HPCG</p>

<?php include 'common/footer.html' ?>

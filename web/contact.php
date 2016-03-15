<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - Contact');
  $page->setNavIdentifier('home');

  $y = <<< END
    th { text-align: left; padding-right: 1em; vertical-align: top;
      padding-bottom: 1em; }
    td { vertical-align: top; padding-bottom: 1em; }
END;
  $page->setInlineStyles($y);

?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="default.php">Home</a> - Contact</div>

<table cellspacing="0" cellpadding="0">

<tr><th>Primary Contacts:</th>
<td><a href="http://www.netlib.org/utk/people/JackDongarra/">Jack Dongarra</a></td></tr>
<td><a href="http://www.cs.sandia.gov/~maherou">Michael Heroux,</a></td>
<td><a href="http://web.eecs.utk.edu/~luszczek/">Piotr Luszczek</a></td></tr>

</table>

<?php include 'common/footer.html' ?>

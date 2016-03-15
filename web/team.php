<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - Team');
  $page->setNavIdentifier('about');

  $y = <<< END
    th { text-align: left; padding-right: 1em; }
END;
  $page->setInlineStyles($y);

?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="about.php">About</a> - Team</div>

<table cellspacing="0" cellpadding="0">

<tr><th>Leaders:&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp </th>
<td><a href="http://www.cs.sandia.gov/~maherou/">Mike Heroux</a>,
<a href="http://www.netlib.org/utk/people/JackDongarra/"> Jack Dongarra</a>,
<a href="http://web.eecs.utk.edu/~luszczek/"> Piotr Luszczek</a></td></tr>

</tr>
</table>
</br>

<?php include 'common/footer.html' ?>

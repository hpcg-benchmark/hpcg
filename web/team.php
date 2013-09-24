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
<td><a href="http://www.netlib.org/utk/people/JackDongarra/">Jack Dongarra</a></td></tr>
<td><a href="http://web.eecs.utk.edu/~luszczek/">Piotr Luszczek</a></td></tr>

</tr>
</table>
</br>
<table>
<tr><th>Contributors:</th>
</td>

</tr>
</table>
</br>
<table>
<tr><th>Logistics:&nbsp&nbsp&nbsp&nbsp</th>
<td><a href="mailto:jmwille@sandia.gov">Jim Willenbring </a></td>

</tr>

</table>
</br>
<h3>Mailing Address:</h3>

<p>Sandia National Laboratories<br/>
P.O. Box 5800, Mail Stop 1320<br/>
Albuquerque, NM 87185-1320</p>

<br/>

<?php include 'common/footer.html' ?>

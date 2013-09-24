<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('Mantevo - Team');
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
<a href="http://www.sandia.gov/~rfbarre/">Richard Barrett</a></td>
</tr>
</table>
</br>
<table>
<tr><th>Contributors:</th>
<td>David Beckingsale,
<a href="mailto:dwbarne@sandia.gov">Daniel Barnette</a>,
Ted Barragy,
Mike Boulton,
</br>Shao-Ping Chen,
<a href="mailto:pscrozi@sandia.gov">Paul Crozier</a>,
<a href="mailto:dwdoerf@sandia.gov">Doug Doerfler</a>,
<a href="mailto:hcedwar@sandia.gov">Carter Edwards</a>,
</br>Wayne Gaudin,
Tim Germann,
<a href="mailto:sdhammo@sandia.gov">Simon Hammond</a>,
Andy Herdman,
</br>Stephen Jarvis,
Justin Lijutens,
<a href="mailto:ptlin@sandia.gov">Paul Lin</a>,
Andrew Mallinson,
</br>Simon McIntosh-Smith,
Jamaludin Mohd-Yusof,
Oliver Perks,
</br>Steve Plimpton,
David Richards,
Christopher Sewell,
Gregg Skinner,
</br><a href="mailto:dstark@sandia.gov">Dylan Stark</a>,
Sriram Swainarayan,
<a href="mailto:hkthorn@sandia.gov">Heidi Thornquist</a>,
Tim Trucano,
</br><a href="mailto:crtrott@sandia.gov">Christian Trott</a>,
<a href="mailto:ctvaugh@sandia.gov">Courtenay Vaughan</a>,
Art Voter,
<a href="mailto:william@sandia.gov">Alan Williams</a></td>

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

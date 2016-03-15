<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - Mail Lists');
  $page->setNavIdentifier('resources');

  $x = <<< END
    table.lists th { text-align: left; padding: .5em 2em .5em 0;
      border-bottom: 1px solid #999; }
    table.lists td { padding: .5em 2em .5em 0; border-bottom: 1px solid #999; }
END;
  $page->setInlineStyles($x);

?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="resources.php">Resources</a> - Mail Lists</div>

<p>HPCG maintains two main lists that are open to the public. The
hpcg-announce list provides a convenient mechanism for distributing
announcements from the development team to the user community. The hpcg-users
list provides a discussion forum for users.
<!--  Both lists are public lists in
that email traffic on the lists is archived here on the open web.-->
</p>

<table class="lists" cellspacing="0" cellpadding="0">
<tr><th>Announce list:</th>
  <td><a href="http://software.sandia.gov/mailman/listinfo/hpcg-announce">subscribe</a></td>
<!--  <td><a href="http://software.sandia.gov/pipermail/hpcg-announce/">archives</a></td>-->
</tr>

<tr><th>Users list:</th>
  <td><a href="http://software.sandia.gov/mailman/listinfo/hpcg-users">subscribe</a></td>
<!--  <td><a href="http://software.sandia.gov/pipermail/hpcg-users/">archives</a></td> -->
</tr>
</table>


<?php include 'common/footer.html' ?>

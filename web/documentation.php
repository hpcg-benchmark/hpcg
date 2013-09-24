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

<p>Reference Documentation for Common Utilities, specifically YAML classes, is <a href="common/html/index.html">here.</a></p>
<p>Each HPCG package maintains its own documentation in the software distribution.</p>

<!--
<ul class="docs">
<li><a href="HerouxBeenThereDoneThat.pdf">Been There, Done That: Lessons Learned from SMP Computing (.pdf)</a></li>
<li><a href="Heroux_GP2_talk.pdf">Using GPUs and CPUs for Engineering Applications: Challenges and Issues (.pdf)</a></li>
<li><a href="SomeThoughtsOnMulticore.pdf">Some Thoughts on Multicore (.pdf)</a></li>
</ul>
<table class="docs" cellspacing="0" cellpadding="0">
<tr><td></td><td>Version 4.1</td><td>Version of the Day</td></tr>
<tr><td>Users Manual</td>
  <td><a href="">PDF</a></td>
  <td><a href="">PDF</a></td></tr>
<tr><td>Reference Manual</td>
  <td><a href="">HTML</a>, <a href="">PDF</a></td>
  <td><a href="">HTML</a></td></tr>
<tr><td>Developers  Manual</td>
  <td><a href="">HTML</a>, <a href="">PDF</a></td>
  <td><a href="">HTML</a></td></tr>
</table>
-->

<?php include 'common/footer.html' ?>

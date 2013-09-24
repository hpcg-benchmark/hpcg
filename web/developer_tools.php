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
<p>svn checkout svn+ssh://username@software.sandia.gov/space/sandiasvn/public/hpcg/trunk HPCG</p>

<!--
<h2>Repository (restricted access)</h2>

<p>DAKOTA's <a href="http://subversion.tigris.org/">subversion</a> code
repository is browsable with <a href="http://viewvc.tigris.org/">ViewVC</a>.
</p>

<p>Browse DAKOTA's 
<a href="https://development.sandia.gov/viewvc/Dakota/">subversion 
repository</a> directly.</p>

<h2>Bugs (restricted access)</h2>

<p><a href="http://www.bugzilla.org/">Bugzilla</a> manages DAKOTA's issue and
requirements database.</p>

<p>Browse DAKOTA's 
<a href="http://development.sandia.gov/bugzilla/">Bugzilla database</a>.</p>

<h2>Mail lists (restricted access)</h2>

<p>Electronic mailing lists and archives are serviced by 
<a href="http://www.gnu.org/software/mailman/index.html">Mailman</a>.</p>

<p>These lists are for internal use by the DAKOTA development team at Sandia
National Laboratories.  They are restricted access in that all email traffic is
archived on Sandia's internal restricted network (not browsable from the open
web).  The purpose of the web archives is to provide a convenient reference and
central database of repository checkin notifications and developer
discussions.</p>

<table class="lists" cellspacing="0" cellpadding="0">
<tr><th>Developers list:</th>
  <td><a href="http://development.sandia.gov/mailman/listinfo/dakota-developers">subscribe</a></td>
  <td><a href="http://development.sandia.gov/pipermail/dakota-developers/">archives</a></td></tr>

<tr><th>Checkins list:</th>
  <td><a href="http://development.sandia.gov/mailman/listinfo/dakota-checkins">subscribe</a></td>
  <td><a href="http://development.sandia.gov/pipermail/dakota-checkins/">archives</a></td></tr>
</table><br/><br/>

-->

<?php include 'common/footer.html' ?>

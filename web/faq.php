<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('Mantevo - FAQ');
  $page->setNavIdentifier('resources');

  $x = <<< END
END;
  $page->setInlineStyles($x); 
  
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="resources.php">Resources</a> - FAQ</div>

<p>None at this point.</p>

<!--
<ul>

<li><a href="#General">General</a>
  <ul>
  <li><a href="#G1">What is DAKOTA?</a>
  <li><a href="#G2">How is DAKOTA used?</a>
  <li><a href="#G3">Why are you releasing DAKOTA as open source?</a>

  <li><a href="#G4">How is it that Sandia can release government 
                     software as open source?</a>
  </ul>
<li><a href="#Support">Support</a>
  <ul>
  <li><a href="#S1">Is support available for DAKOTA?</a>
  <li><a href="#S2">Do you intend to support the Windows environment?</a>

  <li><a href="#S3">Do you intend to support the Macintosh environment?</a>
  </ul>
<li><a href="#Features">Feature Additions</a>
  <ul>
  <li><a href="#F1">Do you have plans for a GUI?</a>
  <li><a href="#F2">How can I contribute?</a>

  </ul>
</ul>

<a name="General"></a><h2>General</h2>

<ol> 

<li><a name="G1"></a><strong>What is DAKOTA?</strong><br/>

DAKOTA is a general-purpose software toolkit for performing systems
analysis and design on high performance computers.  DAKOTA provides
algorithms for design optimization, uncertainty quantification,
parameter estimation, design of experiments, and sensitivity analysis,
as well as a range of parallel computing and simulation interfacing
services.<br/>


<li><a name="G2"></a><strong>How is DAKOTA used?</strong><br/>

To use DAKOTA for a particular application, an interface between
DAKOTA and your simulation code must be developed.  Refer to Sections
1.3 and 16.1 of the <a href="./licensing/release/Users.pdf">Users Manual</a>
for additional information.<br/>

Once the simulation interface has been developed, any of the iterative
studies available in DAKOTA can be performed with your simulation code
through the selection of specifications in a DAKOTA input file.
Refer to the Getting Started chapter in the 
<a href="./licensing/release/Users.pdf">Users Manual</a> for discussion of 
example input files.<br/>


<li><a name="G3"></a><strong>Why are you releasing DAKOTA as open source?</strong><br/>

To foster collaborations and streamline the licensing process.  Of
particular note is the fact that an export control classification of
"publicly available" allows us to work effectively with universities.  
For more on some of the motivations behind open source software in general, 
<a href="http://www.firstmonday.dk/issues/issue3_3/raymond/">The Cathedral 
and the Bazaar</a> is interesting reading.<br/>


<li><a name="G4"></a><strong>How is it that Sandia can release government
software as open source?</strong><br/>

Sandia is a government-owned, contractor-operated (GOCO) national
laboratory operated for the U.S. Department of Energy (DOE) by
Lockheed Martin Corporation.  The authority to release open source
software resides with the DOE, and DAKOTA has gone through a series of
copyright assertion and classification approvals to allow release to
the general public.  Important proponents for the open source release
of Sandia software are the DOE's Accelerated Strategic Computing (ASC) 
Program Office and the DOE's Office of Science.<br/>

</ol>

<a name="Support"></a><h2>Support</h2>

<ol> 

<li><a name="S1"></a><strong>Is support available for DAKOTA?</strong><br/>

The DAKOTA team does not provide commercial support.  However, in
order to maintain a quality open source release, you can expect the
use of modern SQA practices such as version control, unit and
regression testing, and bug tracking.  In addition, DAKOTA has a
regular release schedule for distribution of new features, bug fixes,
and Users/Reference/Developers documentation updates.<br/>

If this is not enough, we are exploring commercialization partners
who can optionally provide training and on-site support for a fee.



<li><a name="S2"></a><strong>Do you intend to support the Windows
environment?</strong><br/>

A Windows port using <a href="http://www.cygwin.com">Cygwin</a> 
has been made available starting with the DAKOTA v3.1 release.<br/>

<li><a name="S3"></a><strong>Do you intend to support the Macintosh
environment?</strong><br/>

A Mac OSX port has been made available starting with the DAKOTA 
v3.2 release.<br/>

</ol>

<a name="Features"></a><h2>Feature Additions</h2>

<ol>

<li><a name="F1"></a><strong>Do you have plans for a GUI?</strong><br/>

Yes.  Version 1.0 beta of JAGUAR has been released with DAKOTA Version 
4.0.  Downloads are available from the main download page, following a 
<a href="http://www.cs.sandia.gov/web1400/1400_download.html">download 
registration</a>.<br/>

<li><a name="F2"></a><strong>How can I contribute?</strong><br/>

Open source software initiatives benefit greatly from extensions
contributed by their user communities.  For example, the 
<a href="./licensing/release_notes.html#v3.1">release notes for DAKOTA v3.1</a>
cite a number of these contributions.  Several ways that you can 
contribute include:<br/>
  <UL>

  <li>First, you can use the code and provide us feedback.  We welcome
      constructive suggestions.<br/>
  <li>Second, if you want to port DAKOTA to another platform or operating
      system, you can share the configuration extensions with us for
      distribution within the user community (refer to the Cygnus configure
      documentation in <TT>Dakota/docs</TT> and the build documentation in
      <TT>README</TT> and <TT>INSTALL</TT>).<br/>
  <li>Third, if you want to add a capability such as a new iterative
      algorithm, surrogate model, or interface protocol, this extension
      typically involves a class derivation along with the definition of
      a few virtual functions (refer to the 
      <a href="./licensing/release/html/index.html">Developers Manual</a> 
      for information on class hierarchies and the structure provided by 
      their base classes).<br/>

  <li>Fourth, if you're interested in becoming a regular contributor,
      we can provide access to the issues and requirements that we
      internally track using <a 
      href="http://bugzilla-cs.sandia.gov/bugzilla/">Bugzilla</a>.<br/>
  </UL>

In each of these cases, your point of contact is the DAKOTA development 
team at <a href="mailto:dakota-developers@development.sandia.gov">dakota-developers</a>.<br/>

</ol>

<a name="Download"></a><h2>Downloading DAKOTA</h2>

<ol>

<li><a name="D1"></a><strong>When I download one of the distributions to my
Windows PC, the tar file extraction fails.</strong><br/>

Windows does not like the "<TT>.tar.gz</TT>" suffix on the DAKOTA
distributions and will rename a distribution with a name like
<TT>Dakota_3_0.Solaris2.8.tar.gz</TT> to something like
<TT>Dakota_3_0_Solaris2.8_tar.tar</TT>.  Attempting to extract files
directly from this latter filename will fail since the file must first
be uncompressed.  The solution is simple: rename the file to the
correct name as listed on the Web site and then proceed with
uncompressing the file (using "<TT>gunzip</TT>") and extracting the
file (using "<TT>tar xvf</TT>") on your Unix machine or emulator (do

<I>not</I> use Winzip as this will also cause problems).  To avoid
this issue entirely, download the distribution directly to your Unix 
machine and bypass Windows.<br/>

<li><a name="D2"></a><strong>When I download one of the VOTD source 
distributions to my UNIX workstation, the tar file extraction fails.</strong><br/>

The VOTD tar files are generated under Red Hat Linux using GNU tar.
The path names for some of the files are rather long which can cause
problems with some platform-specific tar implementations (e.g., Sun
Solaris, SGI Irix).  If this happens, then there are a few possible 
solutions: (1) extract the distribution under Linux and then copy it 
to where it's needed, or (2) locate (using "whereis tar") or 
<a href="http://www.gnu.org/software/tar/tar.html">download/build</a> 
GNU tar on the platform of interest.<br/>

<li><a name="D3"></a><strong>When I download one of the DAKOTA manuals in 
PDF, Acrobat reader fails.</strong><br/>

The problem appears to be with embedded PDF viewers in some browsers,
rather than with the PDF files themselves.  In particular, problems
have been reported when using Acrobat 5.0 from within Internet
Explorer or Netscape, whereas other combinations work fine.  In these
cases, we recommend the following:

  <UL>
  <li>try saving the file to disk and using Acrobat reader outside of
      the browser (bypassing the browser-embedded PDF viewer).
  <li>try another computer/browser/Acrobat combination.
  <li>for the Reference and Developers manuals, you can use the HTML 
      documentation 
      (<a href="./licensing/release/html-ref/index.html">Reference</a>, 
       <a href="./licensing/release/html/index.html">Developers</a>)
      if hardcopies are not needed.<br/>
  </UL>

</ol>
-->

<?php include 'common/footer.html' ?>

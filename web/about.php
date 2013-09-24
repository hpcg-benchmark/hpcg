<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('Mantevo - About');
  $page->setNavIdentifier('about');
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb">About</div>

<h3>About the Mantevo project</h3>

<p>The Mantevo project is an effort to provide open-source software packages for the analysis, prediction and
 improvement of high performance computing applications.</p>

    <p><b>Goals for the project include:</b>
      
    <ul>
      <li>Predict performance of real applications in new situations.</li>
      <li>Aid computer systems design decisions.</li>
      <li>Foster communication between applications, libraries and computer systems developers.</li>
      <li>Guide application and library developers in algorithm and software design choices for new systems.</li>
    <li> Provide open source software to promote informed algorithm, application and architecture decisions in the HPC community.</li>
  </ul>
  </p>
      
    <p><b>Project features:</b>
      
    <ul>
      <li>Miniapplication and minidriver development done by application developers with years of experience.</li>
      <li>Software available as open source.</li>
      <li>Application space coverage includes: Implicit unstructured PDE applications, explicit dynamics (contact), 
          molecular dynamics, circuit simulation.</li>
      <li>Multiple versions of some miniapplications, including MPI, OpenMP, BEC, Qthreads.</li>
  </ul>
  </p>

<h3>Mantevo Overview Paper</h3>

<p><a href=MantevoOverview.pdf> Improving Performance via Mini-applications (Mantevo Overview) </a></p>

      
<br/>


<?php include 'common/footer.html' ?>

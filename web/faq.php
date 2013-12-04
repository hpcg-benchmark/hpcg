<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('HPCG - FAQ');
  $page->setNavIdentifier('faq');
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb">FAQ</div>

    <ul>
	<li><b>FAQ:</b></li>
        <ul>
         <li><b>Will HPCG replace High Performance Linpack (HPL)?</b>: 
			We do not intend to eliminate HPL.  HPCG will provide 
			an alternative ranking of the TOP 500 machines. We
			expect that HPCG will take several years to both mature
			and emerge as a widely-visible metrics.

         <li><b>Isn't HPCG just another version of the STREAMS benchmark?</b>: 
			If the reference version of HPCG is used for performance
			analysis, the fraction of time spent in the (unoptimized) 
			sparse kernels (in particular ComputeSPMV and ComputeSYMGS)
			will be very high, and HPCG performance will be dominated by
			memory system performance.  In this case, the benchmark will
			give rankings that are very similar to STREAMS for computer
			systems that have a good reduction network, or have few nodes.
			<p>
			However, this is true for many benchmarks.  If HPL were executed
			with reference Fortran BLAS kernels, HPL would also look like a
			STREAMS benchmark.
			<p>
			Warnings have been added to HPCG reports to let the benchmarker
			know that performance will be suboptimal when using reference
			kernels.
			
        </ul>
    </ul>

<?php include 'common/footer.html' ?>

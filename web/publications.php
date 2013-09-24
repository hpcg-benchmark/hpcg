<?php
  include 'common/page.php';
  $page = new Page();
  $page->setPathToWebRoot('');
  $page->setPageTitle('Mantevo - Publications');
  $page->setNavIdentifier('resources');
?>

<?php include 'common/header.html' ?>

<div class="breadcrumb"><a href="resources.php">Resources</a> - Publications</div>

<ul class="docs">
<li><a href="MantevoOverview.pdf"> Improving Performance via Mini-applications (Mantevo Overview) (.pdf)</a></li>
<li><a href="HerouxBeenThereDoneThat.pdf">Been There, Done That: Lessons Learned from SMP Computing (.pdf)</a></li>
<li><a href="Heroux_GP2_talk.pdf">Using GPUs and CPUs for Engineering Applications: Challenges and Issues (.pdf)</a></li>
<li><a href="SomeThoughtsOnMulticore.pdf">Some Thoughts on Multicore (.pdf)</a></li>
<li><a href="http://www.iop.org/EJ/abstract/1742-6596/125/1/012035">
                Design issues for numerical libraries on scalable multicore architectures (SciDAC 2008 paper)</a></li>
<li><a href="mantevo_views.pdf">Mantevo Views: A Flexible System for Gathering and Analyzing Data for the Mantevo Project (.pdf)</a></li>
<li><a href="http://dl.acm.org/citation.cfm?id=2148600.2148602&coll=DL&dl=GUIDE&CFID=326785201&CFTOKEN=67857032">Mini-applications: Vehicles for Co-Design (SC11 Best Poster)</a></li>
</ul>
<!--
<ul>
<li><a href="#Books">Book chapters</a></li>
<li><a href="#Journals">Journal Papers</a></li>
<li><a href="#ConfP">Conference Papers</a></li>
<li><a href="#ConfA">Conference Abstracts</a></li>
<li><a href="#Sand">SAND Reports</a></li>
</ul><br/>

<a name="Books"></a><h2>Book chapters</h2>

<ul> 
<li> Eldred, M.S., Bichon, B.J., Adams, B.M., and Mahadevan, S.,
"Overview of Reliability Analysis and Design Capabilities in DAKOTA
with Application to Shape Optimization of MEMS," (to appear),
<cite>Structural Design Optimization Considering Uncertainties</cite>, 
Tsompanakis, Lagaros, and Papadrakakis, eds.<br/><br/>

<li> Biegler, L.T., Ghattas, O., Heinkenschloss, M., and van Bloemen 
Waanders, B., "Large-scale PDE-constrained Optimization: An Introduction," 

<cite>Large-Scale PDE-Constrained Optimization</cite>, Lecture Notes in 
Computational Science and Engineering, No. 30, Springer-Verlag, 2003, 
pp. 3-13.<br/><br/>

<li> Salinger, A.G., Pawlowski, R.P., Shadid, J.N., van Bloemen
Waanders, B., Bartlett, R., Itle, G.C., and Biegler, L., "rSQP
Optimization of Large Scale Reacting Flow Applications with MPSalsa,"
<cite>Large-Scale PDE-Constrained Optimization</cite>, Lecture Notes in 
Computational Science and Engineering, No. 30, Springer-Verlag, 2003, 
pp. 45-59.<br/><br/>

<li> Bartlett, R.A. and Biegler, L.T., 
<a href="./papers/SantaFePDE_rSQP.pdf">
"rSQP++: An Object-Oriented Framework for Successive Quadratic 
Programming,"</a> <cite>Large-Scale PDE-Constrained Optimization</cite>, 
Lecture Notes in Computational Science and Engineering, No. 30, 
Springer-Verlag, 2003, pp. 316-330.<br/><br/>

</ul>

<a name="Journals"></a><h2>Journal Papers</h2>

<ul>

<li> Eldred, M.S., Agarwal, H., Perez, V.M., Wojtkiewicz, S.F., Jr.,
and Renaud, J.E., <a href="./papers/SIE-KM-005.pdf">"Investigation of 
Reliability Method Formulations in DAKOTA/UQ,"</a>
<cite>Structure & Infrastructure Engineering: Maintenance, Management,
Life-Cycle Design & Performance</cite>, Vol. 3, No. 3, Sept. 2007, pp. 199-213.<br/><br/>

   <ul>
   <li> Also <a href="./papers/UQ_Reliability_paper_PMC04.pdf">appears</a>
   in <cite>Proceedings of the 9th ASCE Joint Specialty Conference on 
   Probabilistic Mechanics and Structural Reliability</cite>, Albuquerque, 
   NM, July 26-28, 2004.<br/><br/>
   </ul>

<li> Giunta, A.A., McFarland, J. M., Swiler, L.P., and Eldred, M.S., "The
promise and peril of uncertainty quantification using response surface
approximations," <cite>Structure & Infrastructure Engineering: Maintenance, 
Management, Life-Cycle Design & Performance</cite>, special issue 
on Uncertainty Quantification and Design under Uncertainty of Aerospace 
Systems, Vol. 2, Nos. 3-4, Sept.-Dec. 2006, pp. 175-189.<br/><br/>

<li> Lemke, R.W., Knudson, M.D., Bliss, D.E., Cochrane, K., Davis, J.-P., 
Giunta, A.A., Harjes, H.C., and Slutz, S.A., "Magnetically accelerated, 
ultrahigh velocity flyer plates for shock wave experiments," <cite>J. Applied 
Physics</cite>, Vol. 98, 2005.<br/><br/>

<li> Salinger, A.G., Pawlowski, R.P., Shadid, J.N., and van Bloemen Waanders,
B., <a href="./papers/CVD_2004.pdf">"Computational Analysis and Optimization 
of a Chemical Vapor Deposition Reactor with Large-Scale Computing,"</a> 
Industrial and Engineering Chemistry Research, in press, 2004.<br/><br/>

<li> Simpson, T. W., Booker, A. J., Ghosh, D., Giunta, A. A., Koch,
P. N. and Yang, R.-J., "Approximation Methods in Multidisciplinary
Analysis and Optimization: A Panel Discussion," <cite>Structural and
Multidisciplinary Optimization</cite>, Vol. 27, No. 5, 2004, pp. 302-313.<br/><br/>

<li> Eldred, M.S., Giunta, A.A., and van Bloemen Waanders, B.G., 
<a href="./papers/StrOpt_JPaper.pdf">
"Multilevel Parallel Optimization Using Massively Parallel Structural 
Dynamics,"</a> <cite>Structural and Multidisciplinary Optimization</cite>,
Springer-Verlag, Vol. 27, Nos. 1-2, May 2004, pp. 97-109.<br/><br/>
   <ul>
   <li> Also appears as paper AIAA-2001-1625 in <cite>Proceedings of the 42nd 
   AIAA/ASME/ASCE/AHS/ASC Structures, Structural Dynamics, and Materials 
   Conference</cite>, Seattle, WA, April 16-19, 2001.<br/><br/>

   </ul>

<li> Romero, V. J., Swiler L. P., and Giunta, A. A., "Construction of
Response Surfaces Based on Progressive-Lattice-Sampling Experimental
Designs," <cite>Structural Safety</cite>, Vol. 26, No. 2, pp. 201-219, 
2004.<br/><br/>

<li> Greenberg, D.S., Hart, W.E., and Phillips, C.A., 
<a href="./papers/DeptScaleSC.pdf">
"Enabling Department-Scale Supercomputing,"</a> <cite>Algorithms for 
Parallel Processing</cite>, IMA Volumes in Mathematics and Its 
Applications, 105:321-344. 1999.<br/><br/>

<li> Hart, W.E., 
<a href="http://www.cs.sandia.gov/~wehart/abstracts.html#Har99">
"Sequential Stopping Rules for Random Optimization
Methods with Applications to Multistart Local Search,"</a> SIAM Journal
of Optimization, 1999, pp. 270-290.  Also appears as Sandia
Technical Report SAND94-2666, Nov. 1994.<br/><br/>

<li> Dowding, K.J., and Blackwell, B.F., 
<a href="./papers/mstjoint.pdf"> 
"Joint Experimental/Computational Techniques to Measure Thermal 
Properties of Solids,"</a> <cite>Measurement Science and Technology</cite>, 
Vol. 9, No. 6, June 1998, pp. 877-887.<br/><br/>

<li> Eldred, M.S., Outka, D.E., Bohnhoff, W.J., Witkowski, W.R.,
Romero, V.J., Ponslet, E.R., and Chen, K.S., 
<a href="./papers/CMSE96.html"> 
"Optimization of Complex Mechanics Simulations with Object-Oriented 
Software Design,"</a> <cite>Computer Modeling and Simulation in Engineering</cite>, 
Vol. 1, No. 3, August 1996, pp. 323-352.<br/><br/>
   <ul>
   <li> Also appears as paper AIAA-95-1433 in <cite>Proceedings of the 36th 
   AIAA/ASME/ ASCE/AHE/ASC Structures, Structural Dynamics, and Materials
   Conference</cite>, New Orleans, LA, April 10-13, 1995, pp. 2406-2415.<br/><br/>

   </ul>
</ul>

<a name="ConfP"></a><h2>Conference Papers</h2>

<ul>
<li> Eldred, M.S., Webster, C.G., and Constantine, P., 
"Evaluation of Non-Intrusive Approaches for Wiener-Askey 
Generalized Polynomial Chaos," abstract submitted for 
<cite>49th AIAA/ASME/ASCE/AHS/ASC Structures, Structural Dynamics, 
and Materials Conference (10th AIAA Non-Deterministic Approaches 
Conference)</cite>, Schaumburg, IL, April 7-10, 2008.<br/><br/>

<li> Bichon, B.J., Eldred, M.S., Swiler, L.P., Mahadevan, S., and
McFarland, J.M., <a href="./papers/AIAA-2007-1946.pdf">"Multimodal 
Reliability Assessment for Complex Engineering Applications using 
Efficient Global Optimization,"</a> paper AIAA-2007-1946 in 
<cite>Proceedings of the 48th AIAA/ASME/ASCE/AHS/ASC Structures, 
Structural Dynamics, and Materials Conference (9th AIAA 
Non-Deterministic Approaches Conference)</cite>, Honolulu, HI, April
23-26, 2007.<br/><br/>

<li> Eldred, M.S., Adams, B.M., Copps, K.D., Carnes, B., Notz, P.K.,
Hopkins, M.M., and Wittwer, J.W., <a href="./papers/AIAA-2007-1934.pdf">
"Solution-Verified Reliability Analysis and Design of Compliant 
Micro-Electro-Mechanical Systems,"</a> paper AIAA-2007-1934 in 
<cite>Proceedings of the 48th AIAA/ASME/ASCE/AHS/ASC Structures, 
Structural Dynamics, and Materials Conference (9th AIAA 
Non-Deterministic Approaches Conference)</cite>, Honolulu, HI, April
23-26, 2007.<br/><br/>

<li> Adams, B.M., Eldred, M.S., Wittwer, J., and Massad, J.,
<a href="./papers/AIAA-2006-7000.pdf">
"Reliability-Based Design Optimization for Shape Design of Compliant
Micro-Electro-Mechanical Systems,"</a> paper AIAA-2006-7000
in the <cite>Proceedings of the 11th AIAA/ISSMO Multidisciplinary Analysis 
and Optimization Conference</cite>, Portsmouth, VA, Sept. 6-8, 2006.<br/><br/>

<li> Eldred, M.S. and Dunlavy, D.M., <a href="./papers/AIAA-2006-7117.pdf">
"Formulations for Surrogate-Based
Optimization with Data Fit, Multifidelity, and Reduced-Order Models,"</a>
paper AIAA-2006-7117 in the <cite>Proceedings of the 11th
AIAA/ISSMO Multidisciplinary Analysis and Optimization Conference</cite>,
Portsmouth, VA, Sept. 6-8, 2006.<br/><br/>

<li> Giunta, A.A., Swiler, L.P., Brown, S.L., Eldred, M.S., Richards,
M.D., and Cyr, E.C., <a href="./papers/AIAA-2006-7049.pdf">"The Surfpack 
Software Library for Surrogate Modeling of Sparse Irregularly Spaced 
Multidimensional Data,"</a> paper AIAA-2006-7049 in the <cite>Proceedings 
of the 11th AIAA/ISSMO Multidisciplinary Analysis and Optimization 
Conference</cite>, Portsmouth, VA, Sept. 6-8, 2006.<br/><br/>

<li> Robinson, T.D., Willcox, K.E., Eldred, M.S., and Haimes, R.,
<a href="./papers/AIAA-2006-7114.pdf">
"Multifidelity Optimization for Variable-Complexity Design," </a>
paper AIAA-2006-7114 in the <cite>Proceedings of the 11th AIAA/ISSMO
Multidisciplinary Analysis and Optimization Conference</cite>,
Portsmouth, VA, Sept. 6-8, 2006.<br/><br/>

<li> Eldred, M.S. and Bichon, B.J., 
<a href="./papers/AIAA-2006-1828.pdf">"Second-Order Reliability Formulations 
in DAKOTA/UQ,"</a> paper AIAA-2006-1828 in <cite>Proceedings of the 47th
AIAA/ASME/ASCE/AHS/ASC Structures, Structural Dynamics, and Materials
Conference (8th AIAA Non-Deterministic Approaches Conference)</cite>,
Newport, Rhode Island, May 1 - 4, 2006.<br/><br/>

<li> Robinson, T.D., Eldred, M.S., Willcox, K.E., and Haimes, R.,
<a href="./papers/AIAA-2006-1819.pdf">"Strategies for Multifidelity 
Optimization with Variable Dimensional Hierarchical Models,"</a> 
paper AIAA-2006-1819 in <cite>Proceedings of the
47th AIAA/ASME/ASCE/AHS/ASC Structures, Structural Dynamics, and
Materials Conference (2nd AIAA Multidisciplinary Design Optimization
Specialist Conference)</cite>, Newport, Rhode Island, May 1 - 4, 2006.<br/><br/>

<li> Weickum, G., Eldred, M.S., and Maute, K., 
<a href="./papers/AIAA-2006-2145.pdf">"Multi-point Extended
Reduced Order Modeling For Design Optimization and Uncertainty
Analysis,"</a> paper AIAA-2006-2145 in <cite>Proceedings of the 47th
AIAA/ASME/ASCE/AHS/ASC Structures, Structural Dynamics, and Materials
Conference (2nd AIAA Multidisciplinary Design Optimization Specialist
Conference)</cite>, Newport, Rhode Island, May 1 - 4, 2006.<br/><br/>

<li> Eldred, M.S., Bichon, B.J., and Adams, B.M., <a
href="./papers/Overview_Reliability_color.pdf">
"Overview of Reliability Analysis and Design Capabilities in DAKOTA,"</a> 
<cite>Proceedings of the NSF Workshop on Reliable Engineering Computing (REC 
2006)</cite>, Savannah, GA, February 22-24, 2006.<br/><br/>

<li> Eldred, M.S., Giunta, A.A., and Collis, S.S, <a
href="./papers/SBO_Corr_MAO_paper.pdf">
"Second-Order Corrections for Surrogate-Based Optimization with Model
Hierarchies,"</a> paper AIAA-2004-4457 in <cite>Proceedings of the 10th
AIAA/ISSMO Multidisciplinary Analysis and Optimization Conference</cite>,
Albany, NY, Aug. 30 - Sept. 1, 2004.<br/><br/>

<li> Giunta, A.A., Eldred, M.S., Swiler, L.P., Trucano, T.G., and
Wojtkiewicz, S.F., Jr., <a 
href="./papers/OUU_MAO2004.pdf">"Perspectives
on Optimization Under Uncertainty: Algorithms and Applications"</a> paper 
AIAA-2004-4451 in <cite>Proceedings of the 10th AIAA/ISSMO Multidisciplinary 
Analysis and Optimization Conference</cite>, Albany, NY, Aug. 30 - Sept. 1, 
2004.<br/><br/>

<li> Perez, V.M., Eldred, M.S., and Renaud, J.E., <a
href="./papers/Infeas_TR_MAO2004.pdf">"Solving 
the Infeasible Trust-region Problem Using Approximations,"</a> paper
AIAA-2004-4312 in <cite>Proceedings of the 10th AIAA/ISSMO
Multidisciplinary Analysis and Optimization Conference</cite>, Albany,
NY, Aug. 30 - Sept. 1, 2004.<br/><br/>

<li> Giunta, A.A., Eldred, M.S., and Castro, J.P., <a
href="./papers/UQ_RSM_PMC04.pdf">
"Uncertainty Quantification Using Response Surface Approximations,"</a>
<cite>Proceedings of the 9th ASCE Joint Specialty Conference on
Probabilistic Mechanics and Structural Reliability</cite>, Albuquerque,
NM, July 26-28, 2004.<br/><br/>

<li> Perez, V.M., Eldred, M.S., and Renaud, J.E., <a
href="./papers/rSQP_PMC04.pdf">"An rSQP 
Approach for a Single-Level Reliability Optimization," </a>
<cite>Proceedings of the 9th ASCE Joint Specialty Conference on
Probabilistic Mechanics and Structural Reliability</cite>, Albuquerque,
NM, July 26-28, 2004.<br/><br/>

<li> van Bloemen Waanders, B., "Application of Optimization Methods to
the Calibration of Water Distribution Systems," 
<cite>Proceedings of the World Water and Environmental Resources Congress
(EWRI)</cite>, Salt Lake City, UT, June 27 - July 1, 2004.<br/><br/>

<li> Giunta, A.A., Wojtkiewicz, S.F., Jr., and Eldred, M.S., 

<a href="./papers/sand2003-0209C.pdf">"Overview of
Modern Design of Experiments Methods for Computational Simulations,"</a> paper 
AIAA-2003-0649 in <cite>Proceedings of the 41st AIAA Aerospace Sciences 
Meeting and Exhibit</cite>, Reno, NV, Jan. 6-9, 2003.<br/><br/>

<li> Simpson, T.W., Booker, A.J., Ghosh, D., Giunta, A.A., Koch, P.N., and
Yang, R.-J., <a href="./papers/panel_review.pdf">
"Approximation Methods in Multidisciplinary Analysis and Optimization: 
A Panel Discussion,"</a> <cite>3rd ISSMO/AIAA Internet Conference on
Approximations in Optimization</cite>, Oct. 14-18, 2002.<br/><br/>

<li> Eldred, M.S., Giunta, A.A., Wojtkiewicz, S.F., Jr., and Trucano,
T.G., <a href="./papers/MAO2002paper4875.pdf">

"Formulations for Surrogate-Based Optimization Under Uncertainty,"</a> 
paper AIAA-2002-5585 in <cite>Proceedings of
the 9th AIAA/ISSMO Symposium on Multidisciplinary Analysis and
Optimization</cite>, Atlanta, GA, Sept. 4-6, 2002.<br/><br/>

<li> Giunta, A.A., Eldred, M.S., Trucano, T.G., and Wojtkiewicz, S.F.,
Jr., <a href="./papers/sdm2002_1642.pdf">
"Optimization Under Uncertainty Methods for Computational Shock
Physics Applications,"</a> paper AIAA-2002-1642 in <cite>Proceedings of
the 43rd AIAA/ASME/ASCE/AHS/ASC Structures, Structural Dynamics, and
Materials Conference</cite> (Nondeterministic Approaches Forum), Denver,
CO, April 22-25, 2002.<br/><br/>

<li> Giunta, A. A., 
<a href="./papers/reno2002_0538.pdf">"Use of 
Data Sampling, Surrogate Models, and Numerical Optimization in Engineering 
Design,"</a> paper AIAA-2002-0538 in <cite>Proceedings of the 40th AIAA 
Aerospace Sciences Meeting and Exhibit</cite>, Reno, NV, Jan. 2002.<br/><br/>

<li> Wojtkiewicz, S.F., Jr., Eldred, M.S., Field, R.V., Jr., Urbina, A.,
and Red-Horse, J.R., 
<a href="./papers/SDM2001_uq_paper.pdf">
"A Toolkit For Uncertainty Quantification In
Large Computational Engineering Models,"</a> paper AIAA-2001-1455 in
<cite>Proceedings of the 42nd AIAA/ASME/ASCE/AHS/ASC Structures, Structural 
Dynamics, and Materials Conference</cite>, Seattle, WA, April 16-19, 2001.<br/><br/>

<li> Eldred, M.S., Hart, W.E., Schimel, B.D., and van Bloemen
Waanders, B.G., 
<a href="./papers/MDO2000_paper.pdf">
"Multilevel Parallelism for Optimization on MP Computers: Theory and
Experiment,"</a> paper AIAA-2000-4818 in <cite>Proceedings of the 8th 
AIAA/USAF/NASA/ISSMO Symposium on Multidisciplinary Analysis and
Optimization</cite>, Long Beach, CA, September 6-8, 2000.<br/><br/>

<li> Giunta, A.A., and Eldred, M.S., 
<a href="./papers/MDO2000_sao_paper.pdf">
"Implementation of a Trust Region Model Management Strategy in the
DAKOTA Optimization Toolkit,"</a> paper AIAA-2000-4935 in

<cite>Proceedings of the 8th AIAA/USAF/NASA/ISSMO Symposium on
Multidisciplinary Analysis and Optimization</cite>, Long Beach, CA,
September 6-8, 2000.<br/><br/>

<li> Eldred, M.S., and Schimel, B.D., 
<a href="./papers/WCSMO99_longpaper.pdf">
"Extended Parallelism Models for Optimization on Massively Parallel 
Computers,"</a> paper 16-POM-2 in <cite>Proceedings of the 3rd World 
Congress of Structural and Multidisciplinary Optimization (WCSMO-3)</cite>, 
Amherst, NY, May 17-21, 1999.<br/><br/>

<li> Eldred, M.S., and Hart, W.E., 
<a href="./papers/MDO98_paper.pdf">

"Design and Implementation of Multilevel Parallel Optimization on the 
Intel TeraFLOPS,"</a> paper AIAA-98-4707 in <cite>Proceedings of the 7th 
AIAA/USAF/NASA/ISSMO Symposium on Multidisciplinary Analysis and 
Optimization</cite>, St. Louis, MO, Sept. 2-4, 1998, pp. 44-54.<br/><br/>

<li> Blackwell, B.F., and Eldred, M.S., 
<a href="./papers/97nhtc_2col.pdf"> 
"Application of Reusable Interface Technology for Thermal Parameter 
Estimation,"</a> <cite>Proceedings of the 32nd National Heat Transfer 
Conference</cite>, Vol. 2, Eds. Dulikravitch, G.S., and Woodbury, K.E., 
HTD-Vol. 340, August 1997, pp. 1-8.<br/><br/>

<li> Chen, K.S., and Witkowski, W.R., "Design Optimization of 
Liquid-Distribution Chamber-Slot Dies Using the DAKOTA Toolkit," 
50th Annual Conference of the Society for Imaging Science and 
Technology, Cambridge MA, May 18-23, 1997.<br/><br/>

<li> Hobbs, M. L., "A Global HMX Decomposition Model," 1996 JANNAF
Propulsion Systems Hazards Subcommittee Meeting, Naval Postgraduate
School, Monterey, CA, Nov. 4-8, 1996. <br/><br/>

<li> Eldred, M.S., Hart, W.E., Bohnhoff, W.J., Romero, V.J.,
Hutchinson, S.A., and Salinger, A.G., 
<a href="./papers/MDO96_paper.pdf">
"Utilizing Object-Oriented Design to Build Advanced Optimization
Strategies with Generic Implementation,"</a> paper AIAA-96-4164 in
<cite>Proceedings of the 6th AIAA/USAF/NASA/ISSMO Symposium on
Multidisciplinary Analysis and Optimization</cite>, Bellevue, WA,
Sept. 4-6, 1996, pp. 1568-1582. <br/><br/>

<li> <a name="AD">Moen, C.D., Spence, P.A., Meza, J.C., and Plantenga, 
T.D.,</a> "Automatic Differentiation for Gradient-Based Optimization of
Radiatively Heated Microelectronics Manufacturing Equipment," paper
AIAA-96-4118 in <cite>Proceedings of the 6th AIAA/USAF/NASA/ISSMO
Symposium on Multidisciplinary Analysis and Optimization</cite>,
Bellevue, WA, Sept. 4-6, 1996, pp. 1167-1175. <br/><br/>

<li> Ponslet, E.R., and Eldred, M.S., 
"Discrete Optimization of Isolator Locations for Vibration Isolation 
Systems: an Analytical and Experimental Investigation," paper 
AIAA-96-4178 in <cite>Proceedings of the 6th AIAA/USAF/NASA/ISSMO 
Symposium on Multidisciplinary Analysis and Optimization</cite>, 
Bellevue, WA, Sept. 4-6, 1996, pp. 1703-1716. Also appears as
<a href="./papers/VibIsol_SAND961169.pdf"> 
Sandia Technical Report SAND96-1169</a>, May 1996.<br/><br/>

<li> Hart, W.E., 
<a href="ftp://ftp.cs.sandia.gov/pub/papers/wehart/1997/Har97-foga.ps.gz">
"A Stationary Point Convergence Theory for Evolutionary 
Algorithms,"</a> <cite>Proceedings of Foundations of Genetic Algorithms 4</cite>, 
San Diego, CA, August 3-5, 1996, pp. 325-342. <br/><br/>

<li> Hart, W.E., Baden, S., Belew, R.K., Kohn, S., 
<a href="ftp://ftp.cs.sandia.gov/pub/papers/wehart/opt/ipps96.ps.gz">
"Analysis of the Numerical Effects of Parallelism on a Parallel Genetic 
Algorithm,"</a> <cite>Proceedings of the 10th International Parallel Processing 
Symposium</cite>(IPPS `96), Honolulu, HI, April 15-19, 1996, pp. 606-612.<br/><br/>

<li> Hart, W.E., <a
href="ftp://ftp.cs.sandia.gov/pub/papers/wehart/1996/Har96-ep.ps.gz"> "A
Theoretical Comparison of Evolutionary Algorithms and Simulated
Annealing,"</a> <cite>Proceedings of the Fifth Annual Conference on
Evolutionary Programming</cite> (EP `96), San Diego, CA, February 29 -
March 2, 1996, pp. 147-154.<br/><br/>

<li> Harding, D.C., Eldred, M.S., and Witkowski, W.R.,
"Integration of Finite Element Analysis and Numerical Optimization
Techniques for RAM Transport Package Design," <cite>Proceedings of the
11th International Conference on the Packaging and Transportation of
Radioactive Materials</cite> (PATRAM `95), Las Vegas, NV, Dec. 3-8, 1995.<br/><br/>

<li> Harding, D.C., and Eldred, M.S., "Radioactive Material
Transportation Package Design Using Numerical Optimization
Techniques," <cite>Proceedings of the 1995 Joint ASME/JSME Pressure
Vessels and Piping Conference</cite>, Honolulu, Hawaii, July 23-27, 1995,
Vol. PVP-307, pp. 29-36.<br/><br/>

<li> Romero, V.J., Eldred, M.S., Bohnhoff, W.J., and Outka, D.E.,
"Application of Optimization to the Inverse Problem of Finding the
Worst-Case Heating Configuration in a Fire," <cite>Proceedings of the 9th
International Conference on Numerical Methods in Thermal Problems</cite>,
Atlanta, GA, July 17-21, 1995, Vol. 9, Part 2, pp. 1022-1033.<br/><br/>

<li> Witkowski, W.R., Eldred, M.S., and Harding, D.C., "Integration of
Numerical Analysis Tools for Automated Numerical Optimization of a
Transportation Package Design," <cite>Proceedings of the 5th
AIAA/NASA/USAF/ISSMO Symposium on Multidisciplinary Analysis and
Optimization</cite>, paper AIAA94-4259, Panama City Beach, FL,
Sept. 7-9, 1994.<br/><br/>
</ul>

<a name="ConfA"></a><h2>Conference Abstracts</h2>

<ul>
<li> Carnes, B., Copps, K.D., Eldred, M.S., Adams, B.M., Wittwer, J.W.,
"Coupled a posteriori error estimation and uncertainty quantification
for a nonlinear elasticity MEMS problem," abstract for <cite>SIAM Conference 
on Computational Science and Engineering (CSE07)</cite>, Costa Mesa, CA, 
February 19-23, 2007.<br/><br/>

<li> Dunlavy, D.M. and Eldred, M.S., "Formulations for Surrogate-Based
Optimization Using Data Fit and Multifidelity Models," abstract for 
<cite>SIAM Conference on Computational Science and Engineering (CSE07)</cite>, 
Costa Mesa, CA, February 19-23, 2007.<br/><br/>

<li> Robinson, T.D., Willcox, K.E., Eldred, M.S., and Haimes, R.,
"Multifidelity Optimization for Variable-Complexity Design," abstract 
submitted for <cite>Second International Workshop on
Surrogate Modeling and Space Mapping for Engineering Optimization</cite>,
Lyngby, Denmark, Nov. 9-11, 2006.<br/><br/>

<li> Giunta, A.A., Castro, J.P., Hough, P.D.,Gray, G.A., Eldred, M.S.,
"Multifidelity Modeling Approaches in Simulation-Based Optimization," 
abstract for the <cite>SIAM Conference on Optimization</cite>, Stockholm, 
Sweden, May 15-19, 2005.<br/><br/>

<li> Giunta, A.A., Eldred, M.S., Hough, P.D., and Castro, J.P., 
"Overview of Surrogate-Based Optimization Research and Applications 
at Sandia National Laboratories," abstract for the <cite>Surrogate
Optimization Workshop</cite>, Houston, TX, May 24-25, 2004.<br/><br/>

<li> Giunta, A.A. and Eldred, M.S., "Robust Design Optimization Using
Surrogate Models," abstract for the <cite>Robust Optimization-Directed
Design (RODD) Conference</cite>, Shalimar, FL, April 19-21, 2004.<br/><br/>

<li> Giunta, A.A., Eldred, M.S., Wojtkiewicz, S.F., Jr., Trucano,
T.G., and Castro, J.P., "Surrogate-Based Optimization Methods for 
Engineering Design," abstract in <cite>Proceedings of the Fifth Biennial 
Tri-Laboratory Engineering Conference on Computational Modeling</cite>, 
Santa Fe, NM, October 21-23, 2003.<br/><br/>

<li> Giunta, A. A., and Eldred, M. S., "Surrogate-Based Optimization Under
Uncertainty: Formulations and Applications" abstract in the
<cite>Proceedings of the 18th International Symposium on Mathematical 
Programming</cite>, Copenhagen, Denmark, Aug. 2003.<br/><br/>

<li> Eldred, M.S., Giunta, A.A., Wojtkiewicz, S.F., Jr., and Trucano,
T.G., "Formulations for Surrogate-Based Optimization Under Uncertainty,"
abstract in <cite>Proceedings of the 7th U.S. National 
Congress on Computational Mechanics</cite>, Albuquerque, NM, 
July 28-30, 2003.<br/><br/>

<li> Giunta, A.A. and Eldred, M.S., "Engineering Design Optimization 
Algorithms: Theory and Practice," abstract in <cite>Proceedings of the 7th 
U.S. National Congress on Computational Mechanics</cite>, Albuquerque, NM, 
July 28-30, 2003.<br/><br/>

<li> Eldred, M.S., Giunta, A.A., Wojtkiewicz, S.F., Jr., and Trucano,
T.G., "Surrogate-Based Optimization Under Uncertainty: Status and 
Directions," abstract in <cite>SIAM Conference on Computational 
Science and Engineering. Final Program and Abstracts</cite>, San Diego, 
CA, Feb 10-13, 2003.<br/><br/>

<li> Giunta, A.A. and Eldred, M.S., "Case Studies in Computational 
Engineering Design Optimization: Challenges and Solutions," abstract 
in <cite>SIAM Conference on Computational Science and 
Engineering. Final Program and Abstracts</cite>, 
San Diego, CA, Feb 10-13, 2003.<br/><br/>

<li> Eldred, M.S., "DAKOTA: Virtual Prototyping with Large-Scale 
Engineering Simulations," abstract in <cite>IMA Workshop 4: 
Optimization in Simulation-Based Models</cite>, 
Minneapolis, MN, January 9-16, 2003.<br/><br/>

<li> Eldred, M.S., "The DAKOTA Optimization Framework: Virtual
Prototyping with ASCI-Scale Simulations," abstract in <cite>Proceedings
of the Fourth Biennial Tri-Laboratory Engineering Conference on
Computational Modeling</cite>, Albuquerque, NM, Oct. 23-24, 2001, p. 82.<br/><br/>

<li> Wojtkiewicz, S.F., Jr., Field, R.V., Jr., Eldred, M.S., Red-Horse,
J.R., and Urbina, A., "Uncertainty Quantification in Large
Computational Engineering Models," abstract in <cite>Proceedings of the
Fourth Biennial Tri-Laboratory Engineering Conference on Computational
Modeling</cite>, Albuquerque, NM, Oct. 23-24, 2001, p. 11.<br/><br/>

<li> Hart, W. E., Giunta, A. A., Salinger, A. G., and van Bloemen
Waanders, B., <a
href="./papers/abstract_mopta.pdf">"An
Overview of the Adaptive Pattern Search Algorithm and its Application
to Engineering Optimization Problems,"</a> abstract in <cite>Proceedings
of the McMaster Optimization Conference: Theory and Applications</cite>,
McMaster University, Hamilton, Ontario, Canada, August 2001, p. 20.<br/><br/>

  
<li> Wojtkiewicz, S.F., Jr., Eldred M.S., Field, R.V., Jr., Urbina, A.,
Red-Horse, J.R., and Giunta, A.A., "DAKOTA/UQ: A Toolkit for Uncertainty
Quantification in a Multiphysics, Massively Parallel Computational
Environment," presented as (1) poster at <cite>ODU-NASA Training Workshop
on Nondeterministic Approaches and Their Potential for Future
Aerospace Systems</cite>, held in Langley, Virginia, May 30-31, 2001, (2)
abstract (no proceedings) at <cite>USNCCM VI (Sixth United States
Congress on Computational Mechanics)</cite> held in Dearborn, Michigan,
August 1-3, 2001, and (3) abstract (no proceedings) at <cite>LLNL
Sensitivity Analysis Workshop</cite>, August 16-17, 2001.<br/><br/>

<li> van Bloemen Waanders, B., "Simultaneous Analysis and Design
Optimization of Massively Parallel Simulation Codes using Object
Oriented Framework," abstract for <cite>Tenth SIAM Conference on Parallel
Processing for Scientific Computing</cite>, March 2001.<br/><br/>

<li> Giunta, A.A., "Coupling High-Performance Computing, Optimization, 
and Shock Physics Simulations,"</a> abstract in session CP04 of the
<cite>Final Program of the First SIAM Conference on Computational 
Science and Engineering</cite>, Washington, DC, September 21-23, 2000, 
p. 47.<br/><br/>

<li> Hart, W.E., Eldred, M.S., and Giunta, A.A., "Solving 
mixed-integer nonlinear problems with PICO," abstract in 
proceedings of the 
<cite>17th International Symposium on Mathematical 
Programming (ISMP 2000)</cite>, Atlanta, GA, August 7-11, 2000.<br/><br/>

<li> van Bloemen Waanders, B.G., Eldred, M.S., Hart, W.E., Schimel,
B.D., and Giunta, A.A., "A Review of the Dakota Toolkit, Multilevel
Parallelism for Complex PDE Simulations on TeraFLOP Computers,"
abstract presented in the <cite>Optimization in Engineering
Minisymposium</cite> at the <cite>SIAM Annual Meeting</cite>, Rio Grande,
Puerto Rico, July 10-14, 2000.<br/><br/>

<li> Romero, V.J., Painton, L.A., and Eldred, M.S., "Optimization
Under Uncertainty: Shifting of Maximum Vulnerability Point Due to
Uncertain Failure Thresholds," 1997 INFORMS Spring Meeting, San Diego,
CA, May 1997. <br/><br/>

<li> Eldred, M.S., Outka, D.E., and Bohnhoff, W.J., "Optimization of
Complex Engineering Simulations with the DAKOTA Toolkit," abstract in
<cite>Proceedings of the First Biennial Tri-Laboratory Engineering
Conference on Computational Modeling</cite>, Pleasanton, CA,
Oct. 31-Nov. 2, 1995.<br/><br/>
</ul>

<a name="Sand"></a><h2>SAND Reports</h2>

<ul>
<li>Adams, B.M., Bichon, B.J., Carnes, B., Copps, K.D., Eldred, M.S., 
Hopkins, M.H., Neckels, D.C., Notz, P.K., Subia, S.R., and Wittwer, J.W., 
<a href="./papers/2006_MilestoneFY06_SAND.pdf">"Solution-Verified
Reliability Analysis and Design of Bistable MEMS Using Error Estimation
and Adaptivity,"</a> Sandia Technical Report SAND2006-6286, October
2006.<br/><br/>

<li>Eldred, M.S., Brown, S.L., Adams, B.M., Dunlavy, D.M., Gay, D.M., 
Swiler, L.P., Giunta, A.A., Hart, W.E., Watson, J.-P., Eddy, J.P., 
Griffin, J.D., Hough, P.D., Kolda, T.G., Martinez-Canales, M.L. and 
Williams, P.J., <a href="./licensing/release/Users4.0.pdf">"DAKOTA,
A Multilevel Parallel Object-Oriented Framework for Design
Optimization, Parameter Estimation, Uncertainty Quantification, and
Sensitivity Analysis: Version 4.0 Users Manual,"</a> Sandia Technical
Report SAND2006-6337, October 2006.  Updated September 2007
(<a href="./licensing/release/Users4.1.pdf">Version 4.1</a>).<br/><br/>

<li>Eldred, M.S., Brown, S.L., Adams, B.M., Dunlavy, D.M., Gay, D.M., 
Swiler, L.P., Giunta, A.A., Hart, W.E., Watson, J.-P., Eddy, J.P., 
Griffin, J.D., Hough, P.D., Kolda, T.G., Martinez-Canales, M.L. and 
Williams, P.J., <a href="./licensing/release/Reference4.0.pdf">"DAKOTA,
A Multilevel Parallel Object-Oriented Framework for Design
Optimization, Parameter Estimation, Uncertainty Quantification, and
Sensitivity Analysis: Version 4.0 Reference Manual,"</a> Sandia Technical
Report SAND2006-4055, October 2006.  Updated September 2007
(<a href="./licensing/release/Reference4.1.pdf">Version 4.1</a>).<br/><br/>

<li>Eldred, M.S., Brown, S.L., Adams, B.M., Dunlavy, D.M., Gay, D.M., 
Swiler, L.P., Giunta, A.A., Hart, W.E., Watson, J.-P., Eddy, J.P., 
Griffin, J.D., Hough, P.D., Kolda, T.G., Martinez-Canales, M.L. and 
Williams, P.J., <a href="./licensing/release/Developers4.0.pdf">"DAKOTA,
A Multilevel Parallel Object-Oriented Framework for Design
Optimization, Parameter Estimation, Uncertainty Quantification, and
Sensitivity Analysis: Version 4.0 Developers Manual,"</a> Sandia Technical
Report SAND2006-4056, October 2006.  Updated September 2007
(<a href="./licensing/release/Developers4.1.pdf">Version 4.1</a>).<br/><br/>

<li> Swiler, L.P. and Wyss, G.D., <a href="./papers/LhsManual2004.pdf">"A 
User's Guide to Sandia's Latin Hypercube Sampling Software: LHS UNIX 
Library Standalone Version,"</a> Sandia Technical Report SAND2004-2439, 
July 2004.<br/><br/>

<li> Eldred, M.S., Giunta, A.A., van Bloemen Waanders, B.G.,
Wojtkiewicz, S.F., Jr., Hart, W.E., and Alleva, M.P., 

<a href="./licensing/release/Users3.0.pdf">"DAKOTA, A Multilevel
Parallel Object-Oriented Framework for Design Optimization, Parameter
Estimation, Uncertainty Quantification, and Sensitivity Analysis.
Version 3.0 Users Manual."</a>  Sandia Technical Report SAND2001-3796,
April 2002.  Updated April 2003 
(<a href="./licensing/release/Users3.1.pdf">Version 3.1</a>).<br/><br/>

<li> Eldred, M.S., Giunta, A.A., van Bloemen Waanders, B.G.,
Wojtkiewicz, S.F., Jr., Hart, W.E., and Alleva, M.P., 
<a href="./licensing/release/Reference3.0.pdf">"DAKOTA, A Multilevel
Parallel Object-Oriented Framework for Design Optimization, Parameter
Estimation, Uncertainty Quantification, and Sensitivity Analysis.
Version 3.0 Reference Manual."</a>  Sandia Technical Report SAND2001-3515,
April 2002.  Updated April 2003 
(<a href="./licensing/release/Reference3.1.pdf">Version 3.1</a>),
July 2004 (<a href="./licensing/release/Reference3.2.pdf">Version 3.2</a>), and
December 2004 (<a href="./licensing/release/Reference3.3.pdf">Version 3.3</a>).<br/><br/>

<li> Eldred, M.S., Giunta, A.A., van Bloemen Waanders, B.G.,
Wojtkiewicz, S.F., Jr., Hart, W.E., and Alleva, M.P., 
<a href="./licensing/release/Developers3.0.pdf">"DAKOTA, A Multilevel
Parallel Object-Oriented Framework for Design Optimization, Parameter
Estimation, Uncertainty Quantification, and Sensitivity Analysis.
Version 3.0 Developers Manual."</a>  Sandia Technical Report SAND2001-3514,
April 2002.  Updated April 2003 
(<a href="./licensing/release/Developers3.1.pdf">Version 3.1</a>),
July 2004 (<a href="./licensing/release/Developers3.2.pdf">Version 3.2</a>), and
December 2004 (<a href="./licensing/release/Developers3.3.pdf">Version 3.3</a>).<br/><br/>

<li> van Bloemen Waanders, B., Bartlett, R., Long, K., Boggs, P.,
and Salinger, A., 
<a href="./papers/pdeco_ldrd_finalreport.pdf">

"Large Scale Non-Linear Programming
for PDE Constrained Optimization,"</a> Sandia Technical Report 
SAND2002-3198, October 2002.<br/><br/>

<li> Gardner, D.R., and Vaughan, C.T., 
<a href="http://infoserve.sandia.gov/sand_doc/1999/992953.pdf">
"The Optimization of a Shaped-Charge Design Using Parallel 
Computers,"</a> Sandia Technical Report SAND99-2953, November 1999.<br/><br/>

<li> Hobbs, M.L., Erickson, K.L., and Chu, T.Y., 
<a href="http://infoserve.sandia.gov/sand_doc/1999/992758.pdf">
"Modeling Decomposition of Unconfined Polyurethane Foam,"</a> 

Sandia Technical Report SAND99-2758, November 1999.<br/><br/>

<li> Eisler, G.R., and Veers, P.S., 
<a href="http://infoserve.sandia.gov/sand_doc/1998/982668.pdf">
"Parameter Optimization Applied to Use of Adaptive Blades on a 
Variable Speed Wind Turbine,"</a> Sandia Technical Report SAND98-2668, 
December 1998.<br/><br/>

<li> McGee, B.C., Hobbs, M.L., and Baer, M.R., 
<a href="http://infoserve.sandia.gov/sand_doc/1998/981191.pdf">
"Exponential 6 Parameterization for the JCZ3-EOS,"</a> Sandia 
Technical Report SAND98-1191, July 1998.<br/><br/>

<li> <a name="LDRDfinal">Eldred, M.S., 
<a href="http://infoserve.sandia.gov/sand_doc/1998/980340.pdf">
"Optimization Strategies for Complex Engineering Applications,"</a> 
Sandia Technical Report SAND98-0340, February 1998.<br/><br/>

<li> <a name="GENFIND">Zimmerman, D.C.,</a> "Genetic Algorithms for
Navigating Expensive and Complex Design Spaces," Final Report for
Sandia National Laboratories contract AO-7736 CA 02 (year 2),
Sept. 1996. <br/><br/>

<li> Hart, W.E., 

<a href="http://www.cs.sandia.gov/~wehart/papers.html">
"Evolutionary Pattern Search Algorithms,"</a> Sandia
Technical Report SAND95-2293, October 1995.<br/><br/>

<li> Zimmerman, D.C., "Genetic Algorithms for Navigating Expensive and
Complex Design Spaces," Final Report for Sandia National Laboratories
contract AO-7736 (year 1), Sept. 1995.<br/><br/>

<li> Meza, J.C., and Plantenga, T.D., 
<a href="http://infoserve.sandia.gov/sand_doc/1995/958224.pdf">
"Optimal Control of a CVD Reactor for Prescribed Temperature
 Behavior,"</a> Sandia Technical Report SAND95-8224, April 1995.<br/><br/>

<li> Moen, C.D., Spence, P.A., and Meza, J.C., 
<a href="http://infoserve.sandia.gov/sand_doc/1995/958223.pdf">
"Optimal Heat Transfer Design of Chemical Vapor Deposition 
Reactors,"</a> Sandia Technical Report SAND95-8223, April 1995.<br/><br/>

<li> Meza, J.C., "OPT++: An Object-Oriented Class Library for Nonlinear 
Optimization," Sandia Technical Report SAND94-8225, March 1994.  
</ul>
-->

<?php include 'common/footer.html' ?>

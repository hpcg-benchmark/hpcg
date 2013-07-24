load A.dat
A=spconvert(A);
load x.dat
load xexact.dat
load b.dat
normr = norm(b-A*xexact);
os = sprintf('Residual of b - A*xexact (should be zero) %10.4e',normr);
disp(os);
normr = norm(b-A*x);
os = sprintf('Residual of b - A*x (initial guess)       %10.4e\n\n',normr);
disp(os);
xcomputed = hpcg(A,b,x);
function [x]=hpcg(A, b, x)
iter =0;
maxiters = 100;
p = x;
Ap = A*p;
r = b - Ap;
normr = sqrt(r'*r);
os = sprintf('\n\nInitial residual          = %10.4e',normr);
disp(os);
while norm(r) > 1e-16 && iter < maxiters
iter=iter+1;
z = symgs(A,r);
if iter==1
    p = z;
    rtz = r'*z;
else
    oldrtz = rtz;
    rtz = r'*z;
    beta = rtz/oldrtz;
    p = beta*p + z;
end
Ap = A*p;
pAp = p'*Ap;
alpha = rtz/pAp;
x = x + alpha*p;
r = r - alpha*Ap;
normr = sqrt(r'*r);
os = sprintf('Residual at iteration %3d = %10.4e',iter,normr);
disp(os);
end
end

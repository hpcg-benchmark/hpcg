function [x] = symgs(A,y)
LA = tril(A);
UA = triu(A);
DA = diag(diag(A));
x = LA\y;
x1 = y - LA*x + DA*x;
x = UA\x1;
end

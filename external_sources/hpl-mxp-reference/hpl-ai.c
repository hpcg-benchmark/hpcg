#include "hpl-ai.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

void sgemm(char transa, char transb, int m, int n, int k,
           float alpha, float* A, int lda, float* B, int ldb,
           float beta, float* C, int ldc);

void strsm(char side, char uplo, char transa, char diag, int m, int n,
           float alpha, float* A, int lda, float* B, int ldb);

int main(int argc, char* argv[]) {

    int n = 100;      // matrix size
    int max_it = 50;  // maximum number of iterations in GMRES
    if (argc >= 2) {
        n = atoi(argv[1]);
    }
    if (argc >= 3) {
        max_it = atoi(argv[2]);
    }
    if (max_it >= n) {
        max_it = n - 1;
    }

    double time_convert, time_factor, time_solve, time_gmres, time_total;

    int lda = (n + 16 - 1) / 16 * 16;  // round up to multiple of 16
    unsigned long long iseed = 1;      // RNG seed

    double* A = (double*)malloc(lda * n * sizeof(double));
    double* LU = (double*)malloc(lda * n * sizeof(double));
    double* b = (double*)malloc(n * sizeof(double));
    double* x = (double*)malloc(n * sizeof(double));
    float* sA = (float*)malloc(lda * n * sizeof(float));
    float* sb = (float*)malloc(n * sizeof(float));

    matgen(A, lda, n, iseed);
    vecgen(b, n, iseed+1);

    printf(
        "======================================================================"
        "==========\n");
    printf(
        "                        HPL-AI Mixed-Precision Benchmark              "
        "          \n");
    printf(
        "       Written by Yaohung Mike Tsai, Innovative Computing Laboratory, "
        "UTK       \n");
    printf(
        "======================================================================"
        "==========\n");
    printf("\n");
    printf(
        "This is a reference implementation with the matrix generator, an "
        "example\n");
    printf(
        "mixed-precision solver with LU factorization in single and GMRES in "
        "double,\n");
    printf("as well as the scaled residual check.\n");
    printf(
        "Please visit http://www.icl.utk.edu/research/hpl-ai for more "
        "details.\n");
    printf("\n");

    // Convert A and b to single.
    time_convert = get_wtime();
    time_total = time_convert;
    convert_double_to_float(A, lda, sA, lda, n, n);
    convert_double_to_float(b, n, sb, n, n, 1);
    time_convert = get_wtime() - time_convert;
    printf("Time spent in conversion to single: %12.3f second\n", time_convert);

    // LU factorization without pivoting.
    time_factor = get_wtime();
    time_total = time_factor;
    sgetrf_nopiv(n, n, sA, lda);
    time_factor = get_wtime() - time_factor;
    printf("Time spent in factorization       : %12.3f second\n", time_factor);

    // Forward and backward substitution.
    time_solve = get_wtime();
    strsm('L', 'L', 'N', 'U', n, 1, 1.0, sA, lda, sb, n);
    strsm('L', 'U', 'N', 'N', n, 1, 1.0, sA, lda, sb, n);
    time_solve = get_wtime() - time_solve;
    printf("Time spent in solve               : %12.3f second\n", time_solve);

    // Convert result back to double.
    time_convert = get_wtime();
    convert_float_to_double(sA, lda, LU, lda, n, n);
    convert_float_to_double(sb, n, x, n, n, 1);
    time_convert = get_wtime() - time_convert;
    printf("Time spent in conversion to double: %12.3f second\n", time_convert);

    // Using GMRES without restart.
    time_gmres = get_wtime();
    // GMRES is checking preconditioned residual so the tolerance is smaller.
    double tol = DBL_EPSILON / 2.0 / ((double)n / 4.0);
    gmres(n, A, lda, x, b, LU, lda, max_it, 1, tol);
    time_gmres = get_wtime() - time_gmres;
    time_total = get_wtime() - time_total;
    printf("Time spent in GMRES               : %12.3f second\n", time_gmres);
    printf("Total time                        : %12.3f second\n", time_total);

    double ops = 2.0 / 3.0 * n * n * n + 3.0 / 2.0 * n * n;
    printf("Effective operation per sec       : %12f GFLOPs\n",
           1e-9 * ops / time_total);

    // Check final backward error.
    double norm_A = dlange('I', n, n, A, lda);
    double norm_x = dlange('I', n, 1, x, n);
    double norm_b = dlange('I', n, 1, b, n);
    dgemv('N', n, n, 1.0, A, lda, x, 1, -1.0, b, 1);
    double threshold = 16.0;
    double eps = DBL_EPSILON / 2;
    double error =
        dlange('I', n, 1, b, n) / (norm_A * norm_x + norm_b) / n / eps;
    printf("The following scaled residual check will be computed:\n");
    printf(
        "||Ax-b||_oo / ( eps * ( || x ||_oo * || A ||_oo + || b ||_oo ) * N "
        ")\n");
    printf("The relative machine precision (eps) is taken to be: %e\n", eps);
    printf("Computational tests pass if scaled residuals are less than %.1f\n",
           threshold);
    printf("||Ax-b||_oo/(eps*(||A||_oo*||x||_oo+||b||_oo)*N)= %f ...", error);
    if (error < threshold) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }

    return 0;
}

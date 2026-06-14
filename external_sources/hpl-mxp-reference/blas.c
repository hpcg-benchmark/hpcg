#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "hpl-ai.h"

#define A(i, j) *HPLAI_INDEX2D(A, (i), (j), lda)
#define B(i, j) *HPLAI_INDEX2D(B, (i), (j), ldb)
#define C(i, j) *HPLAI_INDEX2D(C, (i), (j), ldc)

void sgemm(char transa, char transb, int m, int n, int k,
           float alpha, float *A, int lda, float *B, int ldb,
           float beta, float *C, int ldc) {
    int i, j, l;

    // Only supprt transa=='N', trabsb=='N'
    if (transa != 'N' || transb != 'N') {
        printf("Not supported in SGEMM.\n");
        return;
    }

    if (m == 0 || n == 0) {
        return;
    }

    if ((alpha == 0.0 || k == 0) && beta == 1.0) {
        return;
    }

    if (alpha == 0.0) {
        if (beta == 0.0) {
            for (j = 0; j < n; j++) {
                for (i = 0; i < m; i++) {
                    C(i, j) = 0.0;
                }
            }
        } else {
            for (j = 0; j < n; j++) {
                for (i = 0; i < m; i++) {
                    C(i, j) = beta * C(i, j);
                }
            }
        }
    }

    for (j = 0; j < n; j++) {
        if (beta == 0.0) {
            for (i = 0; i < m; i++) {
                C(i, j) = 0.0;
            }
        } else {
            for (i = 0; i < m; i++) {
                C(i, j) = beta * C(i, j);
            }
        }
        for (l = 0; l < k; l++) {
            float temp = alpha * B(l, j);
            for (i = 0; i < m; i++) {
                C(i, j) += temp * A(i, l);
            }
        }
    }
    return;
}
void strsm(char side, char uplo, char transa, char diag, int m, int n,
           float alpha, float *A, int lda, float *B, int ldb) {

    int i, j, k;

    // Only support side=='L', transa=='N', alpha==1.0.
    if (side != 'L' || transa != 'N' || alpha != 1.0) {
        printf("Not supported in STRSM.\n");
        return;
    }

    if (m == 0 || n == 0) {
        return;
    }

    int nounit = diag == 'N';

    if (uplo == 'U') {
        for (j = 0; j < n; j++) {
            for (k = m - 1; k >= 0; k--) {
                if (nounit) {
                    B(k, j) = B(k, j) / A(k, k);
                }
                for (i = 0; i < k; i++) {
                    B(i, j) = B(i, j) - B(k, j) * A(i, k);
                }
            }
        }
    } else {
        for (j = 0; j < n; j++) {
            for (k = 0; k < m; k++) {
                if (nounit) {
                    B(k, j) = B(k, j) / A(k, k);
                }
                for (i = k + 1; i < m; i++) {
                    B(i, j) = B(i, j) - B(k, j) * A(i, k);
                }
            }
        }
    }
    return;
}
void dtrsm(char side, char uplo, char transa, char diag, int m, int n,
           double alpha, double *A, int lda, double *B, int ldb) {
    int i, j, k;

    // Only support side=='L', transa=='N', alpha==1.0.
    if (side != 'L' || transa != 'N' || alpha != 1.0) {
        printf("Not supported in DTRSM.\n");
        return;
    }

    if (m == 0 || n == 0) {
        return;
    }

    int nounit = diag == 'N';

    if (uplo == 'U') {
        for (j = 0; j < n; j++) {
            for (k = m - 1; k >= 0; k--) {
                if (nounit) {
                    B(k, j) = B(k, j) / A(k, k);
                }
                for (i = 0; i < k; i++) {
                    B(i, j) = B(i, j) - B(k, j) * A(i, k);
                }
            }
        }
    } else {
        for (j = 0; j < n; j++) {
            for (k = 0; k < m; k++) {
                if (nounit) {
                    B(k, j) = B(k, j) / A(k, k);
                }
                for (i = k + 1; i < m; i++) {
                    B(i, j) = B(i, j) - B(k, j) * A(i, k);
                }
            }
        }
    }
    return;
}

double dlange(char norm, int m, int n, double *A, int lda) {
    int i, j;

    // Frobenius norm
    if (norm == 'F') {
        double sum = 0.0;
        for (j = 0; j < n; ++j) {
            for (i = 0; i < m; ++i) {
                sum += A(i, j) * A(i, j);
            }
        }
        return sqrt(sum);
        // Infinity norm
    } else if (norm == 'I') {
        double *work = (double *)malloc(m * sizeof(double));
        memset(work, 0, m * sizeof(double));
        double max = 0.0;
        for (j = 0; j < n; ++j) {
            for (i = 0; i < m; ++i) {
                work[i] += fabs(A(i, j));
            }
        }
        for (i = 0; i < m; ++i) {
            if (max < work[i]) {
                max = work[i];
            }
        }
        free(work);
        return max;
    }
    return 0;
}

void dgemv(char trans, int m, int n, double alpha, double *A,
           int lda, double *X, int incx, double beta, double *Y,
           int incy) {
    int i, j;
    if (trans != 'N' || incx != 1 || incy != 1) {
        return;
    }

    if (beta != 1.0) {
        if (beta == 0.0) {
            for (i = 0; i < m; ++i) {
                Y[i] = 0;
            }
        } else {
            for (i = 0; i < m; ++i) {
                Y[i] = beta * Y[i];
            }
        }
    }

    if (alpha == 0.0) {
        return;
    }

    for (j = 0; j < n; ++j) {
        double temp = alpha * X[j];
        for (i = 0; i < m; ++i) {
            Y[i] += temp * A(i, j);
        }
    }
    return;
}

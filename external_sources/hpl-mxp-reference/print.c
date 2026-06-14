#include <stdio.h>

#include "hpl-ai.h"

#define A(i, j) *HPLAI_INDEX2D(A, (i), (j), lda)

void print_matrix_float(float *A, int lda, int m, int n) {

    int i, j;

    if (lda < m) {
        return;
    }
    printf("[%s", m==1 ? " " : "\n");
    for (i = 0; i < m; ++i) {
        for (j = 0; j < n; ++j) {
            printf(" %10.6f", A(i, j));
        }
        printf("%s", m>1 ? "\n" : " ");
    }
    printf("];\n");
    return;
}

void print_matrix_double(double *A, int lda, int m, int n) {

    int i, j;

    if (lda < m) {
        return;
    }
    printf("[%s", m==1 ? "" : "\n");
    for (i = 0; i < m; ++i) {
        for (j = 0; j < n; ++j) {
            printf(" %14.10e", A(i, j));
        }
        printf("%s", m>1 ? "\n" : " ");
    }
    printf("];\n");
    return;
}

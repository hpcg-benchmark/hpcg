#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "hpl-ai.h"

#define A(i, j) *HPLAI_INDEX2D(A, (i), (j), lda)

#define LCG_A 6364136223846793005ULL
#define LCG_C 1ULL
#define LCG_MUL 5.4210108624275222e-20f

#define MCG_A 14647171131086947261ULL
#define MCG_MUL 2.328306436538696e-10f

// Linear Congruential Generator (LCG).
inline unsigned long long lcg_rand(unsigned long long *piseed) {
    *piseed = *piseed * LCG_A + LCG_C;
    return *piseed;
}

// LCG jump ahead function to go through N steps in log(N) time.
inline void lcg_advance(unsigned int delta, unsigned long long *piseed) {
    unsigned long long accum_a = LCG_A;
    unsigned long long accum_c = LCG_C;
    while (delta != 0) {
        if (delta & 1) {
            delta = delta - 1;
            *piseed = *piseed * accum_a + accum_c;
        }
        delta = delta / 2;
        accum_c *= accum_a + LCG_C;
        accum_a *= accum_a;
    }
}

// Generate double floating-point number from uniform(-0.5, 0.5) from LCG.
inline double lcg_rand_double(unsigned long long *piseed) {
    return ((double)lcg_rand(piseed)) * LCG_MUL - 0.5;
}

// Multiplicative Congruential Generator (MCG). Seed should be odd number.
inline unsigned long int mcg_rand(unsigned long long *piseed) {
    *piseed *= MCG_A;
    return *piseed >> 32; /* use high 32 bits */
}

// Jump ahead function to go through N steps in log(N) time.
inline void mcg_advance(unsigned int delta, unsigned long long *piseed) {
    unsigned long long accum = MCG_A;
    while (delta != 0) {
        if (delta & 1) {
            delta = delta - 1;
            *piseed *= accum;
        }
        delta = delta / 2;
        accum = accum * accum;
    }
}

// Generate double floating-point number from uniform(-0.5, 0.5) from MCG.
inline double mcg_rand_double(unsigned long long *piseed) {
    return ((double)mcg_rand(piseed)) * MCG_MUL - 0.5;
}

// Generate a row diagonally dominant square matrix A.
void matgen(double *A, int lda, int m, unsigned long long iseed) {

    int i, j;

    double *diag = (double *)malloc(m * sizeof(double));
    memset(diag, 0, m * sizeof(double));

    for (j = 0; j < m; j++) {
        for (i = 0; i < m; i++) {
            A(i, j) = lcg_rand_double(&iseed);
            diag[i] += fabs(A(i, j));
        }
    }

    for (i = 0; i < m; i++) {
        A(i, i) = diag[i] - fabs(A(i, i));
    }

    free(diag);
}

void vecgen(double *v, int n, unsigned long long iseed) {
    int i;
    for (i = 0; i < n; i++) {
        v[i] = lcg_rand_double(&iseed);
    }
    return;
}

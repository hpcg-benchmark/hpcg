/* 
 * -- High Performance Computing Linpack Benchmark (HPL)                
 *    HPL - 2.3 - December 2, 2018                          
 *    Antoine P. Petitet                                                
 *    University of Tennessee, Knoxville                                
 *    Innovative Computing Laboratory                                 
 *    (C) Copyright 2000-2008 All Rights Reserved                       
 *                                                                      
 * -- Copyright notice and Licensing terms:                             
 *                                                                      
 * Redistribution  and  use in  source and binary forms, with or without
 * modification, are  permitted provided  that the following  conditions
 * are met:                                                             
 *                                                                      
 * 1. Redistributions  of  source  code  must retain the above copyright
 * notice, this list of conditions and the following disclaimer.        
 *                                                                      
 * 2. Redistributions in binary form must reproduce  the above copyright
 * notice, this list of conditions,  and the following disclaimer in the
 * documentation and/or other materials provided with the distribution. 
 *                                                                      
 * 3. All  advertising  materials  mentioning  features  or  use of this
 * software must display the following acknowledgement:                 
 * This  product  includes  software  developed  at  the  University  of
 * Tennessee, Knoxville, Innovative Computing Laboratory.             
 *                                                                      
 * 4. The name of the  University,  the name of the  Laboratory,  or the
 * names  of  its  contributors  may  not  be used to endorse or promote
 * products  derived   from   this  software  without  specific  written
 * permission.                                                          
 *                                                                      
 * -- Disclaimer:                                                       
 *                                                                      
 * THIS  SOFTWARE  IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING,  BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY
 * OR  CONTRIBUTORS  BE  LIABLE FOR ANY  DIRECT,  INDIRECT,  INCIDENTAL,
 * SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES  (INCLUDING,  BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA OR PROFITS; OR BUSINESS INTERRUPTION)  HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT,  STRICT LIABILITY,  OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 * ---------------------------------------------------------------------
 */ 
/*
 * Include files
 */
#include "hpl.h"

#ifndef HPL_dswap

#ifdef STDC_HEADERS
void HPL_dswap
(
   const int                        N,
   double *                         X,
   const int                        INCX,
   double *                         Y,
   const int                        INCY
)
#else
void HPL_dswap
( N, X, INCX, Y, INCY )
   const int                        N;
   double *                         X;
   const int                        INCX;
   double *                         Y;
   const int                        INCY;
#endif
{
/* 
 * Purpose
 * =======
 *
 * HPL_dswap swaps the vectors x and y.
 * 
 *
 * Arguments
 * =========
 *
 * N       (local input)                 const int
 *         On entry, N specifies the length of the vectors  x  and  y. N
 *         must be at least zero.
 *
 * X       (local input/output)          double *
 *         On entry,  X  is an incremented array of dimension  at  least
 *         ( 1 + ( n - 1 ) * abs( INCX ) )  that  contains the vector x.
 *         On exit, the entries of the incremented array  X  are updated
 *         with the entries of the incremented array Y.
 *
 * INCX    (local input)                 const int
 *         On entry, INCX specifies the increment for the elements of X.
 *         INCX must not be zero.
 *
 * Y       (local input/output)          double *
 *         On entry,  Y  is an incremented array of dimension  at  least
 *         ( 1 + ( n - 1 ) * abs( INCY ) )  that  contains the vector y.
 *         On exit, the entries of the incremented array  Y  are updated
 *         with the entries of the incremented array X.
 *
 * INCY    (local input)                 const int
 *         On entry, INCY specifies the increment for the elements of Y.
 *         INCY must not be zero.
 *
 * ---------------------------------------------------------------------
 */ 
#ifdef HPL_CALL_CBLAS
   cblas_dswap( N, X, INCX, Y, INCY );
#endif
#ifdef HPL_CALL_VSIPL
   register double           x0, x1, x2, x3, y0, y1, y2, y3;
   double                    * StX;
   register int              i;
   int                       nu;
   const int                 incX2 = 2 * INCX, incY2 = 2 * INCY,
                             incX3 = 3 * INCX, incY3 = 3 * INCY,
                             incX4 = 4 * INCX, incY4 = 4 * INCY;

   if( N > 0 )
   {
      if( ( nu = ( N >> 2 ) << 2 ) != 0 )
      {
         StX = X + nu * INCX;
 
         do
         {
            x0 = (*X);      y0 = (*Y);      x1 = X[INCX ];  y1 = Y[INCY ];
            x2 = X[incX2];  y2 = Y[incY2];  x3 = X[incX3];  y3 = Y[incY3];
            *Y        = x0; *X        = y0; Y[INCY ]  = x1; X[INCX ]  = y1;
            Y[incY2]  = x2; X[incX2]  = y2; Y[incY3]  = x3; X[incX3]  = y3;
            X += incX4; Y += incY4;
 
         } while( X != StX );
      }
 
      for( i = N - nu; i != 0; i-- )
      { x0  = (*X); y0  = (*Y); *Y = x0; *X = y0; X += INCX; Y += INCY; }
   }
#endif
#ifdef HPL_CALL_FBLAS
#ifdef HPL_USE_F77_INTEGER_DEF
   const F77_INTEGER         F77N = N, F77incx = INCX, F77incy = INCY;
#else
#define F77N                 N
#define F77incx              INCX
#define F77incy              INCY
#endif
   F77dswap( &F77N, X, &F77incx, Y, &F77incy );
#endif
/*
 * End of HPL_dswap
 */
}
 
#endif

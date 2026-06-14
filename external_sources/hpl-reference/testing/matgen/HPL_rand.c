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

#ifdef STDC_HEADERS
double HPL_rand( void )
#else
double HPL_rand()
#endif
{
/* 
 * Purpose
 * =======
 *
 * HPL_rand generates  the next number  in the  random  sequence.  This
 * function  ensures  that this number lies in the interval (-0.5, 0.5].
 *  
 * The static array irand contains the information (2 integers) required
 * to generate the  next number  in the sequence  X(n).  This  number is
 * computed as X(n) = (2^32 * irand[1] + irand[0]) / d - 0.5,  where the
 * constant d is the largest 64 bit positive unsigned integer. The array
 * irand is then  updated  for the generation of the next number  X(n+1)
 * in  the  random   sequence  as   follows  X(n+1) = a * X(n) + c.  The
 * constants a and c should have been preliminarily stored in the arrays
 * ias and ics as 2 pairs of integers.  The initialization of  ias,  ics
 * and  irand  is performed by the function HPL_setran.
 *
 * ---------------------------------------------------------------------
 */ 
/*
 * .. Local Variables ..
 */
   int                        j[2];
/* ..
 * .. Executable Statements ..
 */
   HPL_setran( 3, j );
/*
 * return number between -0.5 and 0.5
 */
   return( HPL_HALF -
           (((j[0] & 65535) + ((unsigned)j[0] >> 16) * HPL_POW16) / HPL_DIVFAC * HPL_HALF +
           (j[1] & 65535) + ((unsigned)j[1] >> 16) * HPL_POW16) / HPL_DIVFAC * HPL_HALF );
/*
 * End of HPL_rand
 */
}

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
void HPL_ladd
(
   int *                            J,
   int *                            K,
   int *                            I
)
#else
void HPL_ladd
( J, K, I )
   int *                            J;
   int *                            K;
   int *                            I;
#endif
{
/* 
 * Purpose
 * =======
 *
 * HPL_ladd adds  without carry two long positive integers  K and J and
 * puts the result into I. The long integers  I, J, K are encoded on 64
 * bits using an array of 2 integers.  The 32-lower bits  are stored in
 * the  first  entry  of each array,  the 32-higher bits  in the second
 * entry.
 *
 * Arguments
 * =========
 *
 * J       (local input)                 int *
 *         On entry, J is an integer array of dimension 2 containing the
 *         encoded long integer J.
 *
 * K       (local input)                 int *
 *         On entry, K is an integer array of dimension 2 containing the
 *         encoded long integer K.
 *
 * I       (local output)                int *
 *         On entry, I is an integer array of dimension 2. On exit, this
 *         array contains the encoded long integer result.
 *
 * ---------------------------------------------------------------------
 */ 
/*
 * .. Local Variables ..
 */
   unsigned int        itmp0, itmp1;
   unsigned int        ktmp0 = K[0] & 65535, ktmp1 = (unsigned)K[0] >> 16;
   unsigned int        ktmp2 = K[1] & 65535, ktmp3 = (unsigned)K[1] >> 16;
   unsigned int        jtmp0 = J[0] & 65535, jtmp1 = (unsigned)J[0] >> 16;
   unsigned int        jtmp2 = J[1] & 65535, jtmp3 = (unsigned)J[1] >> 16;

/* ..
 * .. Executable Statements ..
 */
/*
 *    K[1] K[0] K  I[0]  = (K[0]+J[0]) % 2^32
 *    XXXX XXXX    carry = (K[0]+J[0]) / 2^32
 *
 * +  J[1] J[0] J  I[1] = K[1] + J[1] + carry
 *    XXXX XXXX    I[1] = I[1] % 2^32
 *    -------------
 *    I[1] I[0]
 *    0XXX XXXX I
 */
   itmp0 = ktmp0 + jtmp0;
   itmp1 = itmp0 >> 16;         I[0] = itmp0 - (itmp1 << 16 );
   itmp1 += ktmp1 + jtmp1;      I[0] |= (itmp1 & 65535) << 16;
   itmp0 = (itmp1 >> 16) + ktmp2 + jtmp2;
   I[1] = itmp0 - ((itmp0 >> 16 ) << 16);
   itmp1 = (itmp0 >> 16) + ktmp3 + jtmp3;
   I[1] |= (itmp1 & 65535) << 16;
/*
 * End of HPL_ladd
 */
}

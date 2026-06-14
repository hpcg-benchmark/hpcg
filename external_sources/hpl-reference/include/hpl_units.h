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
 */ 
#ifndef HPL_UNITS_H
#define HPL_UNITS_H
/*
 * ---------------------------------------------------------------------
 * Include files
 * ---------------------------------------------------------------------
 */
#include "hpl_pmisc.h"
#include "hpl_pauxil.h"
/*
 * ---------------------------------------------------------------------
 * #define macro constants
 * ---------------------------------------------------------------------
 */
#define    HPL_MAXROUT       50
#define    HPL_MAXRNAME      15

#define    HPL_TRUE         'T'
#define    HPL_FALSE        'F'

#define    HPL_INDXG2P_ROUT   "HPL_indxg2p"
#define    HPL_INDXG2L_ROUT   "HPL_indxg2l"
#define    HPL_INDXL2G_ROUT   "HPL_indxl2g"
#define    HPL_NUMROC_ROUT    "HPL_numroc"
#define    HPL_NUMROCI_ROUT   "HPL_numrocI"
/*
 * ---------------------------------------------------------------------
 * Function prototypes
 * ---------------------------------------------------------------------
 */
void            HPL_unit_info
STDC_ARGS(
(  FILE * *,        int *,           int *,           int *,
   int *,           int *,           int *,           int *,
   int *,           int *,           int *,           char [][HPL_MAXRNAME],
   int [] ) );
 
void            HPL_unit_indxg2l
STDC_ARGS(
(  FILE *,          const int,       const int,       const int,
   const int,       const int,       const int,       const int,
   const int,       long *,          long * ) );
int             HPL_chek_indxg2l
STDC_ARGS(
(  FILE *,          const char *,    const int,       const int,
   const int,       const int,       const int,       const int,
   const int,       long *,          long * ) );
 
void            HPL_unit_indxl2g
STDC_ARGS(
(  FILE *,          const int,       const int,       const int,
   const int,       const int,       const int,       const int,
   const int,       long *,          long * ) );
int             HPL_chek_indxl2g
STDC_ARGS(
(  FILE *,          const char *,    const int,       const int,
   const int,       const int,       const int,       const int,
   const int,       long *,          long * ) );
 
void            HPL_unit_indxg2p
STDC_ARGS(
(  FILE *,          const int,       const int,       const int,
   const int,       const int,       const int,       const int,
   const int,       long *,          long * ) );
int             HPL_chek_indxg2p
STDC_ARGS(
(  FILE *,          const char *,    const int,       const int,
   const int,       const int,       const int,       const int,
   const int,       long *,          long * ) );
 
void            HPL_unit_numroc
STDC_ARGS(
(  FILE *,          const int,       const int,       const int,
   const int,       const int,       const int,       const int,
   const int,       long *,          long * ) );
void            HPL_unit_numrocI
STDC_ARGS(
(  FILE *,          const int,       const int,       const int,
   const int,       const int,       const int,       const int,
   const int,       const int,       long *,          long * ) );
int             HPL_chek_numrocI
STDC_ARGS(
(  FILE *,          const char *,    const int,       const int,
   const int,       const int,       const int,       const int,
   const int,       const int,       long *,          long * ) );

#endif
/*
 * End of hpl_units.h
 */

/*---------------------------------------------------------------------------*/
/*!
 * \file   array_operations.h
 * \author Wayne Joubert
 * \date   Thu Jan 16 15:39:53 EST 2014
 * \brief  Functions to operate on multi-dim arrays, header.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
/*---------------------------------------------------------------------------*/

#ifndef _array_operations_h_
#define _array_operations_h_

#include "env.h"
#include "definitions.h"
#include "dimensions.h"
#include "quantities.h"

#ifdef __cplusplus_IGNORE
extern "C"
{
#endif

/*===========================================================================*/
/*---Initialize state vector to required input value---*/

void initialize_state( P* const RESTRICT       v,
                       const Dimensions        dims,
                       const int               nu,
                       const Quantities* const quan );

/*===========================================================================*/
/*---Initialize state vector to zero---*/

void initialize_state_zero( P* const RESTRICT     v,
                            const Dimensions      dims,
                            const int             nu );

/*===========================================================================*/
/*---Compute vector norm info for state vector---*/

void get_state_norms( const P* const RESTRICT     vi,
                      const P* const RESTRICT     vo,
                      const Dimensions            dims,
                      const int                   nu,
                      P* const RESTRICT           normsqp,
                      P* const RESTRICT           normsqdiffp,
                      Env* const                  env );

/*===========================================================================*/
/*---Copy vector---*/

void copy_vector(       P* const RESTRICT     vo,
                  const P* const RESTRICT     vi,
                  const size_t                n );

/*===========================================================================*/

#ifdef __cplusplus_IGNORE
} /*---extern "C"---*/
#endif

#endif /*---_array_operations_h_---*/

/*---------------------------------------------------------------------------*/

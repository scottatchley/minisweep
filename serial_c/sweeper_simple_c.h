/*---------------------------------------------------------------------------*/
/*!
 * \file   sweeper_simple_c.h
 * \author Wayne Joubert
 * \date   Wed Jan 15 16:06:28 EST 2014
 * \brief  Definitions for performing a sweep, simple version.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
/*---------------------------------------------------------------------------*/

#ifndef _serial_c__sweeper_simple_c_h_
#define _serial_c__sweeper_simple_c_h_

#include "definitions.h"
#include "quantities.h"
#include "array_accessors.h"
#include "array_operations.h"
#include "memory.h"
#include "sweeper_simple.h"

/*===========================================================================*/
/*---Pseudo-constructor for Sweeper struct---*/

void Sweeper_ctor( Sweeper*    sweeper,
                   Dimensions  dims )
{
  /*---Allocate arrays---*/

  sweeper->v_local = pmalloc( dims.na * NU );
  sweeper->facexy  = pmalloc( dims.nx * dims.ny * dims.ne * dims.na *
                                             NU * Sweeper_num_face_octants() );
  sweeper->facexz  = pmalloc( dims.nx * dims.nz * dims.ne * dims.na *
                                             NU * Sweeper_num_face_octants() );
  sweeper->faceyz  = pmalloc( dims.ny * dims.nz * dims.ne * dims.na *
                                             NU * Sweeper_num_face_octants() );
}

/*===========================================================================*/
/*---Pseudo-destructor for Sweeper struct---*/

void Sweeper_dtor( Sweeper* sweeper )
{
  /*---Deallocate arrays---*/

  pfree( sweeper->v_local );
  pfree( sweeper->facexy );
  pfree( sweeper->facexz );
  pfree( sweeper->faceyz );

  sweeper->v_local = 0;
  sweeper->facexy  = 0;
  sweeper->facexz  = 0;
  sweeper->faceyz  = 0;
}

/*===========================================================================*/
/*---Perform a sweep---*/

void Sweeper_sweep(
  Sweeper*         sweeper,
  P* __restrict__  vo,
  P* __restrict__  vi,
  Quantities       quan,
  Dimensions       dims )
{
  assert( sweeper );
  assert( vi );
  assert( vo );

  /*---Declarations---*/
  int ix = 0;
  int iy = 0;
  int iz = 0;
  int ie = 0;
  int im = 0;
  int ia = 0;
  int iu = 0;
  int octant = 0;

  /*---Initialize result array to zero---*/

  initialize_state_zero( vo, dims, NU );

  /*---Loop over octants---*/

  for( octant=0; octant<NOCTANT; ++octant )
  {
    const int octant_ind = 0;
    assert( octant_ind >= 0 && octant_ind < Sweeper_num_face_octants() );

    /*---Decode octant directions from octant number---*/

    const int idirx = Dir_x( octant );
    const int idiry = Dir_y( octant );
    const int idirz = Dir_z( octant );

    /*---Initialize faces---*/

    /*---The semantics of the face arrays are as follows.
         On entering a cell for a solve at the gridcell level,
         the face array is assumed to have a value corresponding to
         "one cell lower" in the relevant direction.
         On leaving the gridcell solve, the face has been updated
         to have the flux at that gridcell.
         Thus, the face is initialized at first to have a value
         "one cell" outside of the domain, e.g., for the XY face,
         either -1 or dims.nx.
         Note also that the face initializer functions now take
         coordinates for all three spatial dimensions --
         the third dimension is used to denote whether it is the
         "lower" or "upper" face and also its exact location
         in that dimension.
    ---*/

    {
      iz = idirz == Dir_up() ? -1 : dims.nz;
      for( iu=0; iu<NU; ++iu )
      for( iy=0; iy<dims.ny; ++iy )
      for( ix=0; ix<dims.nx; ++ix )
      for( ie=0; ie<dims.ne; ++ie )
      for( ia=0; ia<dims.na; ++ia )
      {
        *ref_facexy( sweeper->facexy, dims, NU, ix, iy, ie, ia, iu, octant_ind )
                    = Quantities_init_facexy( ix, iy, iz, ie, ia, iu, dims );
      }
    }

    {
      iy = idiry == Dir_up() ? -1 : dims.ny;
      for( iu=0; iu<NU; ++iu )
      for( iz=0; iz<dims.nz; ++iz )
      for( ix=0; ix<dims.nx; ++ix )
      for( ie=0; ie<dims.ne; ++ie )
      for( ia=0; ia<dims.na; ++ia )
      {
        *ref_facexz( sweeper->facexz, dims, NU, ix, iz, ie, ia, iu, octant_ind )
                    = Quantities_init_facexz( ix, iy, iz, ie, ia, iu, dims );
      }
    }

    {
      ix = idirx == Dir_up() ? -1 : dims.nx;
      for( iu=0; iu<NU; ++iu )
      for( iz=0; iz<dims.nz; ++iz )
      for( iy=0; iy<dims.ny; ++iy )
      for( ie=0; ie<dims.ne; ++ie )
      for( ia=0; ia<dims.na; ++ia )
      {
        *ref_faceyz( sweeper->faceyz, dims, NU, iy, iz, ie, ia, iu, octant_ind )
                    = Quantities_init_faceyz( ix, iy, iz, ie, ia, iu, dims );
      }
    }

    /*---Loop over energy groups---*/

    for( ie=0; ie<dims.ne; ++ie )
    {
      /*---Calculate spatial loop extents---*/

      int ixbeg = idirx==Dir_up() ? 0 : dims.nx-1;
      int iybeg = idiry==Dir_up() ? 0 : dims.ny-1;
      int izbeg = idirz==Dir_up() ? 0 : dims.nz-1;

      int ixend = idirx==Dir_dn() ? 0 : dims.nx-1;
      int iyend = idiry==Dir_dn() ? 0 : dims.ny-1;
      int izend = idirz==Dir_dn() ? 0 : dims.nz-1;

      /*---Loop over gridcells, in proper direction---*/

    for( iz=izbeg; iz!=izend+Dir_inc(idirz); iz+=Dir_inc(idirz) )
    for( iy=iybeg; iy!=iyend+Dir_inc(idiry); iy+=Dir_inc(idiry) )
    for( ix=ixbeg; ix!=ixend+Dir_inc(idirx); ix+=Dir_inc(idirx) )
    {

      /*--------------------*/
      /*---Transform state vector from moments to angles---*/
      /*--------------------*/

      /*---This loads values from the input state vector,
           does the small dense matrix-vector product,
           and stores the result in a relatively small local
           array that is hopefully small enough to fit into
           processor cache.
      ---*/

      for( iu=0; iu<NU; ++iu )
      for( ia=0; ia<dims.na; ++ia )
      {
        P result = P_zero();
        for( im=0; im<dims.nm; ++im )
        {
          result += *ref_a_from_m( quan.a_from_m, dims, im, ia ) *
                    *ref_state( vi, dims, NU, ix, iy, iz, ie, im, iu );
        }
        *ref_v_local( sweeper->v_local, dims, NU, ia, iu ) = result;
      }

      /*--------------------*/
      /*---Perform solve---*/
      /*--------------------*/

      Quantities_solve( sweeper->v_local,
                        sweeper->facexy, sweeper->facexz, sweeper->faceyz,
                        ix, iy, iz, ie, octant, octant_ind, quan, dims );

      /*--------------------*/
      /*---Transform state vector from angles to moments---*/
      /*--------------------*/

      /*---Perform small dense matrix-vector products and store
           the result in the output state vector.
      ---*/

      for( iu=0; iu<NU; ++iu )
      for( im=0; im<dims.nm; ++im )
      {
        P result = P_zero();
        for( ia=0; ia<dims.na; ++ia )
        {
          result += *ref_m_from_a( quan.m_from_a, dims, im, ia ) *
                    *ref_v_local( sweeper->v_local, dims, NU, ia, iu );
        }
        *ref_state( vo, dims, NU, ix, iy, iz, ie, im, iu ) += result;
      }

    } /*---ix/iy/iz---*/

    } /*---ie---*/

  } /*---octant---*/

} /*---sweep---*/

/*===========================================================================*/

#endif /*---_serial_c__sweeper_simple_c_h_---*/

/*---------------------------------------------------------------------------*/

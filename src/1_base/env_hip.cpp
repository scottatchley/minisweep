/*---------------------------------------------------------------------------*/
/*!
 * \file   env_cuda.c
 * \author Wayne Joubert
 * \date   Tue Apr 22 17:03:08 EDT 2014
 * \brief  Environment settings for cuda.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
/*---------------------------------------------------------------------------*/

#include <stddef.h>
#include <stdlib.h>

#include "types.h"
#include "env_types.h"
#include "env_assert.h"
#include "arguments.h"
#include "env_cuda.h"

#ifdef __cplusplus_IGNORE
extern "C"
{
#endif


/*===========================================================================*/
/*---Error handling---*/

Bool_t Env_hip_last_call_succeeded()
{
  Bool_t result = Bool_true;

#ifdef USE_HIP
  /*---NOTE: this read of the last error is a destructive read---*/
  hipError_t error = hipGetLastError();

  if ( error != hipSuccess )
  {
      result = Bool_false;
      printf( "CUDA error detected: %s\n", hipGetErrorString( error ) );
  }
#endif

  return result;
}

/*===========================================================================*/
/*---Initialize CUDA---*/

void Env_hip_initialize_( Env *env, int argc, char** argv )
{
#ifdef USE_HIP
  hipStreamCreate( & env->stream_send_block_ );
  Assert( Env_hip_last_call_succeeded() );

  hipStreamCreate( & env->stream_recv_block_ );
  Assert( Env_hip_last_call_succeeded() );

  hipStreamCreate( & env->stream_kernel_faces_ );
  Assert( Env_hip_last_call_succeeded() );
#endif
}

/*===========================================================================*/
/*---Finalize CUDA---*/

void Env_hip_finalize_( Env* env )
{
#ifdef USE_HIP
  hipStreamDestroy( env->stream_send_block_ );
  Assert( Env_hip_last_call_succeeded() );

  hipStreamDestroy( env->stream_recv_block_ );
  Assert( Env_hip_last_call_succeeded() );

  hipStreamDestroy( env->stream_kernel_faces_ );
  Assert( Env_hip_last_call_succeeded() );
#endif
}

/*===========================================================================*/
/*---Set values from args---*/

void Env_hip_set_values_( Env *env, Arguments* args )
{
#ifdef USE_HIP
  env->is_using_device_ = Arguments_consume_int_or_default( args,
                                             "--is_using_device", Bool_false );
  Insist( env->is_using_device_ == 0 ||
          env->is_using_device_ == 1 ? "Invalid is_using_device value." : 0 );
#endif
}

/*===========================================================================*/
/*---Determine whether using device---*/

Bool_t Env_hip_is_using_device( const Env* const env )
{
#ifdef USE_HIP
  return env->is_using_device_;
#else
  return Bool_false;
#endif
}

/*===========================================================================*/
/*---Memory management, for CUDA and all platforms ex. MIC---*/

#ifndef __MIC__

int* malloc_host_int( size_t n )
{
  Assert( n+1 >= 1 );
  int* result = (int*)malloc( n * sizeof(int) );
  Assert( result );
  return result;
}

/*---------------------------------------------------------------------------*/

P* malloc_host_P( size_t n )
{
  Assert( n+1 >= 1 );
  P* result = (P*)malloc( n * sizeof(P) );
  Assert( result );
  return result;
}

/*---------------------------------------------------------------------------*/

P* malloc_host_pinned_P( size_t n )
{
  Assert( n+1 >= 1 );

  P* result = NULL;

#ifdef USE_HIP
  hipHostMalloc( &result, n==0 ? ((size_t)1) : n*sizeof(P) );
  Assert( Env_hip_last_call_succeeded() );
#else
  result = (P*)malloc( n * sizeof(P) );
#endif
  Assert( result );

  return result;
}

/*---------------------------------------------------------------------------*/

P* malloc_device_P( size_t n )
{
  Assert( n+1 >= 1 );

  P* result = NULL;

#ifdef USE_HIP
  hipMalloc( &result, n==0 ? ((size_t)1) : n*sizeof(P) );
  Assert( Env_hip_last_call_succeeded() );
  Assert( result );
#endif

  return result;
}

/*---------------------------------------------------------------------------*/

void free_host_int( int* p )
{
  Assert( p );
  free( (void*) p );
}

/*---------------------------------------------------------------------------*/

void free_host_P( P* p )
{
  Assert( p );
  free( (void*) p );
}

/*---------------------------------------------------------------------------*/

void free_host_pinned_P( P* p )
{
  Assert( p );
#ifdef USE_HIP
  hipHostFree( p );
  Assert( Env_hip_last_call_succeeded() );
#else
  free( (void*) p );
#endif
}

/*---------------------------------------------------------------------------*/

void free_device_P( P* p )
{
#ifdef USE_HIP
  hipFree( p );
  Assert( Env_hip_last_call_succeeded() );
#endif
}

#endif /*---__MIC__---*/

/*---------------------------------------------------------------------------*/

void cuda_copy_host_to_device_P( P*     p_d,
                                 P*     p_h,
                                 size_t n )
{
#ifdef USE_HIP
  Assert( p_d );
  Assert( p_h );
  Assert( n+1 >= 1 );

  hipMemcpy( p_d, p_h, n*sizeof(P), hipMemcpyHostToDevice );
  Assert( Env_hip_last_call_succeeded() );
#endif
}

/*---------------------------------------------------------------------------*/

void cuda_copy_device_to_host_P( P*     p_h,
                                 P*     p_d,
                                 size_t n )
{
#ifdef USE_HIP
  Assert( p_h );
  Assert( p_d );
  Assert( n+1 >= 1 );

  hipMemcpy( p_h, p_d, n*sizeof(P), hipMemcpyDeviceToHost );
  Assert( Env_hip_last_call_succeeded() );
#endif
}

/*---------------------------------------------------------------------------*/

void cuda_copy_host_to_device_stream_P( P*       p_d,
                                        P*       p_h,
                                        size_t   n,
                                        Stream_t stream )
{
#ifdef USE_HIP
  Assert( p_d );
  Assert( p_h );
  Assert( n+1 >= 1 );

  hipMemcpyAsync( p_d, p_h, n*sizeof(P), hipMemcpyHostToDevice, stream );
  Assert( Env_hip_last_call_succeeded() );
#endif
}

/*---------------------------------------------------------------------------*/

void cuda_copy_device_to_host_stream_P( P*       p_h,
                                        P*       p_d,
                                        size_t   n,
                                        Stream_t stream )
{
#ifdef USE_HIP
  Assert( p_h );
  Assert( p_d );
  Assert( n+1 >= 1 );

  hipMemcpyAsync( p_h, p_d, n*sizeof(P), hipMemcpyDeviceToHost, stream );
  Assert( Env_hip_last_call_succeeded() );
#endif
}

/*===========================================================================*/
/*---Stream management---*/

Stream_t Env_hip_stream_send_block( Env* env )
{
#ifdef USE_HIP
  return env->stream_send_block_;
#else
  return 0;
#endif
}

/*---------------------------------------------------------------------------*/

Stream_t Env_hip_stream_recv_block( Env* env )
{
#ifdef USE_HIP
  return env->stream_recv_block_;
#else
  return 0;
#endif
}

/*---------------------------------------------------------------------------*/

Stream_t Env_hip_stream_kernel_faces( Env* env )
{
#ifdef USE_HIP
  return env->stream_kernel_faces_;
#else
  return 0;
#endif
}

/*---------------------------------------------------------------------------*/

void Env_hip_stream_wait( Env* env, Stream_t stream )
{
#ifdef USE_HIP
  hipStreamSynchronize( stream );
  Assert( Env_hip_last_call_succeeded() );
#endif
}

/*===========================================================================*/

#ifdef __cplusplus_IGNORE
} /*---extern "C"---*/
#endif

/*---------------------------------------------------------------------------*/

#==============================================================================

cmake_minimum_required(VERSION 2.8)

project("minisweep")

SET(INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/src/1_base
  ${CMAKE_SOURCE_DIR}/src/2_sweeper_base
  ${CMAKE_SOURCE_DIR}/src/3_sweeper
  ${CMAKE_SOURCE_DIR}/src/4_driver
  )

SET(SOURCES
  src/1_base/arguments.cpp
  src/1_base/env.cpp
  src/1_base/env_assert.cpp
  src/1_base/env_hip.cpp
  src/1_base/env_mpi.cpp
  src/1_base/pointer.cpp
  src/2_sweeper_base/array_operations.cpp
  src/2_sweeper_base/dimensions.cpp
  src/3_sweeper/faces_kba.cpp
  src/3_sweeper/quantities.cpp
  src/3_sweeper/stepscheduler_kba.cpp
  src/3_sweeper/sweeper.cpp
  src/3_sweeper/sweeper_kernels.cpp
  src/4_driver/runner.cpp
  )

SET(HIP_SOURCES)
FOREACH(FILE IN LISTS SOURCES)
  SET(HIP_SOURCES ${HIP_SOURCES} ${FILE})
ENDFOREACH()

#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -Werror")

IF(USE_MPI)
  find_package(MPI REQUIRED)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_MPI")
ENDIF()

IF(USE_HIP)
  find_package(HIP REQUIRED)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_HIP")
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -DUSE_HIP")
  STRING(REPLACE " " ";" CMAKE_C_FLAGS_ADJUSTED ${CMAKE_C_FLAGS})
  SET(HIPCC_FLAGS "${HIPCC_FLAGS};${CMAKE_C_FLAGS_ADJUSTED}")
#  STRING(REGEX MATCH " -DNM_VALUE=[0-9]*" NM_VALUE_DEF_ " ${CMAKE_C_FLAGS} ")
#  STRING(REPLACE " " ";" NM_VALUE_DEF "${NM_VALUE_DEF_}")
#  SET(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS}${NM_VALUE_DEF};-DUSE_CUDA")
  INCLUDE_DIRECTORIES(${INCLUDE_DIRS})
  ADD_LIBRARY(sweeper STATIC ${HIP_SOURCES})
  ADD_EXECUTABLE(sweep src/4_driver/sweep.cpp)
  TARGET_LINK_LIBRARIES(sweep sweeper)
  ADD_EXECUTABLE(tester src/4_driver/tester.cpp)
  TARGET_LINK_LIBRARIES(tester sweeper)
ELSE()
  INCLUDE_DIRECTORIES(${INCLUDE_DIRS})
  ADD_LIBRARY(sweeper STATIC ${SOURCES})
  ADD_EXECUTABLE(sweep src/4_driver/sweep.c)
  TARGET_LINK_LIBRARIES(sweep sweeper)
  ADD_EXECUTABLE(tester src/4_driver/tester.c)
  TARGET_LINK_LIBRARIES(tester sweeper)
ENDIF()

install(TARGETS sweep DESTINATION bin)
#install(TARGETS tester DESTINATION bin)

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)

IF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -Wmissing-prototypes -Dinline=")
ENDIF()

IF(CMAKE_C_COMPILER_ID STREQUAL "Intel")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Dinline=__forceinline -D__restrict__=restrict -restrict")
ENDIF()

MESSAGE("Compiler ID: ${CMAKE_C_COMPILER_ID}")

# enable_testing()
# add_test(test1 tester)

#==============================================================================

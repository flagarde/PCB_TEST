########################################################
# cmake file for building Marlino
# @author Lagarde François, IPNL
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
########################################################
# project name
project(Proto)
# project version
SET( GRPC_Analysor_VERSION_MAJOR 0 )
SET( GRPC_Analysor_VERSION_MINOR 1 )
SET( GRPC_Analysor_VERSION_PATCH 0 )
include(CheckIncludeFiles)
include(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
if(COMPILER_SUPPORTS_CXX14)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
else()
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX0X)
  if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  else()
    if(COMPILER_SUPPORTS_CXX0X)
      CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else()
      message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please update your *$!%+£ C++ compiler.")
    endif() 
  endif()
endif()
set(PROJECT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/include/")
set(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build/lib/")
SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/build/include/)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fPIC" )
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/ROOT/)
list( APPEND CMAKE_MODULE_PATH $ENV{ROOTSYS}/etc/cmake/)
find_package(ROOT REQUIRED)
include_directories(${ROOT_INCLUDE_DIR})

ADD_SUBDIRECTORY( src/Proto )

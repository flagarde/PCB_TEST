# project name
project(proto)

# project version
SET( ${PROJECT_NAME}_VERSION_MAJOR 0 )
SET( ${PROJECT_NAME}_VERSION_MINOR 1 )
SET( ${PROJECT_NAME}_VERSION_PATCH 0 )
find_package( ZLIB REQUIRED )
### SUB DIRECTORIES #########################################################
#ADD_SUBDIRECTORY( src )
    
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
 
# add the binary tree to the search path for include files
include_directories("${PROJECT_BINARY_DIR}")
include_directories(include)
file(COPY ./include/ DESTINATION ${PROJECT_INCLUDE_DIR})

# -------------------------------------------------
AUX_SOURCE_DIRECTORY( ./src library_sources )
LIST( REMOVE_ITEM library_sources ./src/main.cpp )

ADD_LIBRARY( Protoo STATIC ${library_sources} )
TARGET_LINK_LIBRARIES( Protoo ${CMAKE_DL_LIBS} ${ZLIB_LIBRARIES} ${ROOT_LIBRARIES})
INSTALL( TARGETS Protoo LIBRARY DESTINATION ${LIBRARY_OUTPUT_PATH} ARCHIVE DESTINATION ${LIBRARY_OUTPUT_PATH})
# ----- Marlin executable ----------------------------------------------------
ADD_EXECUTABLE( Read ./src/main.cpp)
# avoid name conflict between Marlin library and Marlin application
TARGET_LINK_LIBRARIES( Read Protoo
${ROOT_LIBRARIES} ${ZLIB_LIBRARIES})
INSTALL( TARGETS Read DESTINATION ${EXECUTABLE_OUTPUT_PATH})
# ----------------------------------------------------------------------------    
    
    
    
    
    



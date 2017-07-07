CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)
PROJECT(offlineanalysis)

# Version number
SET(OfflineAnalysis_VERSION_MAJOR 6)
SET(OfflineAnalysis_VERSION_MINOR 0)

# configure a header file to pass some of the CMake settings
# to the source code
CONFIGURE_FILE(
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )

# add the binary tree to the search path for include files
# so that we will find header files
INCLUDE_DIRECTORIES( BEFORE include )

# Use C++11
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Be strict when compiling in debug mode
SET(CMAKE_CXX_FLAGS_DEBUGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

# needed dependencies
FIND_PACKAGE(ROOT REQUIRED)
INCLUDE_DIRECTORIES( ${ROOT_INCLUDE_DIRS} )
LINK_LIBRARIES( ${ROOT_LIBRARIES} )
ADD_DEFINITIONS ( ${ROOT_DEFINITIONS} )
LIST( APPEND ${PROJECT_NAME}_DEPENDS_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} )
LIST( APPEND ${PROJECT_NAME}_DEPENDS_LIBRARY_DIRS ${ROOT_LIBRARY_DIRS} )
LIST( APPEND ${PROJECT_NAME}_DEPENDS_LIBRARIES ${ROOT_LIBRARIES} )

# add the executable
SET(SOURCE_FILES src/MsgSvc.cc src/utils.cc src/IniFile.cc src/Mapping.cc)
SET(SOURCE_FILES ${SOURCE_FILES} src/RPCDetector.cc src/GIFTrolley.cc src/Infrastructure.cc)
SET(SOURCE_FILES ${SOURCE_FILES} src/RPCHit.cc src/Cluster.cc)
SET(SOURCE_FILES ${SOURCE_FILES} src/OfflineAnalysis.cc src/Current.cc)
SET(SOURCE_FILES ${SOURCE_FILES} src/main.cc)
ADD_EXECUTABLE(offlineanalysis ${SOURCE_FILES})

# add the install targets
INSTALL (TARGETS offlineanalysis DESTINATION bin)

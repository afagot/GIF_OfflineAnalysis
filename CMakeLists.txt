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
INCLUDE_DIRECTORIES( BEFORE ${PROJECT_SOURCE_DIR}/include )

# Use C++11
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Be strict when compiling in debug mode
SET(CMAKE_CXX_FLAGS_DEBUGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

# We provide a module to find the CAEN VME library
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# needed dependencies
FIND_PACKAGE(ROOT REQUIRED)
INCLUDE_DIRECTORIES( ${ROOT_INCLUDE_DIRS} )
LINK_LIBRARIES( ${ROOT_LIBRARIES} )
ADD_DEFINITIONS ( ${ROOT_DEFINITIONS} )
LIST( APPEND ${PROJECT_NAME}_DEPENDS_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} )
LIST( APPEND ${PROJECT_NAME}_DEPENDS_LIBRARY_DIRS ${ROOT_LIBRARY_DIRS} )
LIST( APPEND ${PROJECT_NAME}_DEPENDS_LIBRARIES ${ROOT_LIBRARIES} )

# add the executable
SET(SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/MsgSvc.cc ${PROJECT_SOURCE_DIR}/src/utils.cc ${PROJECT_SOURCE_DIR}/src/IniFile.cc ${PROJECT_SOURCE_DIR}/src/Mapping.cc)
SET(SOURCE_FILES ${SOURCE_FILES} ${PROJECT_SOURCE_DIR}/src/RPCDetector.cc ${PROJECT_SOURCE_DIR}/src/GIFTrolley.cc ${PROJECT_SOURCE_DIR}/src/Infrastructure.cc)
SET(SOURCE_FILES ${SOURCE_FILES} ${PROJECT_SOURCE_DIR}/src/RPCHit.cc ${PROJECT_SOURCE_DIR}/src/Cluster.cc)
SET(SOURCE_FILES ${SOURCE_FILES} ${PROJECT_SOURCE_DIR}/src/OfflineAnalysis.cc ${PROJECT_SOURCE_DIR}/src/Current.cc)
SET(SOURCE_FILES ${SOURCE_FILES} ${PROJECT_SOURCE_DIR}/src/main.cc)
ADD_EXECUTABLE(offlineanalysis ${SOURCE_FILES})

# add the install targets
INSTALL (TARGETS offlineanalysis DESTINATION ${PROJECT_SOURCE_DIR}/bin)

###################
# Common settings #
###################

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)
PROJECT(offlineanalysis)
SET(OfflineAnalysis_VERSION_MAJOR 6)
SET(OfflineAnalysis_VERSION_MINOR 0)

# Use C++11
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Be strict when compiling in debug mode
SET(CMAKE_CXX_FLAGS_DEBUGS "${CMAKE_CXX_FLAGS} -Wall -Werror")


###################
#  Dependencies   #
###################



###################
#   Executables   #
###################

SET(SOURCE_FILES ${SOURCE_FILES} main.cc)
SET(SOURCE_FILES ${SOURCE_FILES} MsgSvc.cc IniFile.cc Mapping.cc utils.cc)
SET(SOURCE_FILES ${SOURCE_FILES} RPCHit.cc Cluster.cc)
SET(SOURCE_FILES ${SOURCE_FILES} RPCDetector.cc GIFTrolley.cc Infrastructure.cc)
SET(SOURCE_FILES ${SOURCE_FILES} OfflineAnalysis.cc Current.cc)

ADD_EXECUTABLE(offlineanalysis ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(offlineanalysis "${CMAKE_THREAD_LIBS_INIT}")


PROJECT(keyword)
cmake_minimum_required(VERSION 2.8)
FILE(REMOVE build)
FILE(MAKE_DIRECTORY build)
FILE(MAKE_DIRECTORY build/log)

ADD_DEFINITIONS(-DLEVELDB_PLATFORM_POSIX)

# include directories
INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}/include
  /usr/local/include
  /usr/include
  /usr/include/openssl
  ${PROJECT_SOURCE_DIR}/brotli/include 
  ${PROJECT_SOURCE_DIR}/gumbo/src 
  ${PROJECT_SOURCE_DIR}/jieba/include
  ${PROJECT_SOURCE_DIR}/jieba/include/limonp  
  ${PROJECT_SOURCE_DIR}/json
  ${PROJECT_SOURCE_DIR}/tinyxml
  ./
)

# lib directories
LINK_DIRECTORIES(
  /usr/local/lib
  /usr/lib  
)

FILE(GLOB_RECURSE XML_FILES  ${CMAKE_SOURCE_DIR}/tinyxml/*.cpp)
FILE(GLOB_RECURSE BROTLI_FILES  ${CMAKE_SOURCE_DIR}/brotli/*.c)
FILE(GLOB_RECURSE GUMBO_FILES_C  ${CMAKE_SOURCE_DIR}/gumbo/*.c)
FILE(GLOB_RECURSE GUMBO_FILES_CC  ${CMAKE_SOURCE_DIR}/gumbo/*.cc)
SET (GUMBO_FILES ${GUMBO_FILES_C} ${GUMBO_FILES_CC} ${BROTLI_FILES}) 
list(APPEND BASE_SDK m glog pthread leveldb curl iconv crypto z ssl) 
SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb3 -std=c99 ")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -ggdb3 -std=c++14")
AUX_SOURCE_DIRECTORY(. DIR_SRCS)
ADD_EXECUTABLE(${PROJECT_NAME} ${DIR_SRCS} ${GUMBO_FILES} ${XML_FILES} )
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${BASE_SDK})



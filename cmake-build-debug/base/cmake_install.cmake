# Install script for directory: /home/bbkgl/CLionProjects/bbkgl/base

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/bbkgl/CLionProjects/bbkgl/cmake-build-debug/base/libmuduo_base.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/muduo/base" TYPE FILE FILES
    "/home/bbkgl/CLionProjects/bbkgl/base/AsyncLogging.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/Atomic.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/BlockingQueue.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/BoundedBlockingQueue.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/Condition.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/CountDownLatch.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/CurrentThread.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/Date.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/Exception.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/FileUtil.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/GzipFile.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/LogFile.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/LogStream.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/Logging.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/Mutex.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/ProcessInfo.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/Singleton.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/StringPiece.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/Thread.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/ThreadLocal.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/ThreadLocalSingleton.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/ThreadPool.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/TimeZone.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/Timestamp.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/Types.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/WeakCallback.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/copyable.h"
    "/home/bbkgl/CLionProjects/bbkgl/base/noncopyable.h"
    )
endif()


cmake_minimum_required(VERSION 3.10)
project(bbkgl)

set(CMAKE_CXX_STANDARD 11)

# Add the source in project root directory
aux_source_directory(. DIRSRCS)

# Add header file include directories
include_directories(./)

add_executable(bbkgl ${DIRSRCS} Channel.cpp Channel.h Poller.cpp Poller.h Timestamp.cpp Timestamp.h Timer.cpp Timer.h
        Callbacks.h TimerId.h TimerQueue.cpp TimerQueue.h EventLoopThread.cpp EventLoopThread.h base/SocketsOpts.cpp
        base/SocketsOpts.h base/InetAddress.cpp base/InetAddress.h base/Socket.cpp base/Socket.h Acceptor.cpp Acceptor.h
        test/test7_2.cpp)

target_link_libraries(bbkgl)

find_package(Threads REQUIRED)
target_link_libraries(bbkgl Threads::Threads)
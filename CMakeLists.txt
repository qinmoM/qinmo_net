cmake_minimum_required(VERSION 3.10)
project(qinmo_net)



set(QINMO_LOG_PATH "${CMAKE_BINARY_DIR}/logs/logger.log"
    CACHE STRING "qinmo-net log path"
)
set(QINMO_LOG_LEVEL 3
    CACHE INTERNAL "qinmo-net log level"
)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/logs)



if (UNIX)
    add_library(qinmo_net
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Channel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/EpollPoller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/EventLoop.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/EventLoopThread.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/EventLoopThreadPool.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/InetAddr.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PacketBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PollerConfig.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ReactorTcpConnection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ReactorTcpServer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SocketTCP.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SocketUDP.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/TcpConnect.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/TcpListen.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Thread.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ThreadConfig.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/TimerManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Timestamp.cpp
    )
elseif (WIN32)
    add_library(qinmo_net
        ${CMAKE_CURRENT_SOURCE_DIR}/src/InetAddr.cpp
        # ${CMAKE_CURRENT_SOURCE_DIR}/src/PacketBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SocketTCP.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SocketUDP.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/TcpConnect.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/TcpListen.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Thread.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ThreadConfig.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Timestamp.cpp
    )
endif()



target_compile_features(qinmo_net
    PUBLIC
        cxx_std_11
)

target_include_directories(qinmo_net
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (UNIX)
    target_link_libraries(qinmo_net
        PUBLIC
            pthread
    )
elseif (WIN32)
    target_link_libraries(qinmo_net
        PUBLIC
            ws2_32
    )
endif()

target_compile_definitions(qinmo_net
    PUBLIC
        QINMO_LOG_PATH="${QINMO_LOG_PATH}"
    PUBLIC
        QINMO_LOG_LEVEL=${QINMO_LOG_LEVEL}
)
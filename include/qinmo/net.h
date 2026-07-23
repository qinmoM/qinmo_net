/**
 * @file <qinmo/net.h>
 * @brief A Simple Net Library
 * @details This module covers:
 *      business-layer           : ReactorTCPServer & ReactorTCPConnection
 *      Low-level                : SocketTCP & SocketUDP
 *      Common non-net utilities : println, StringView, Logger ...
 * 
 * @version 3.1.0
 * @author qinmoM
 * @github https://github.com/qinmoM/qinmo_net
 */
#pragma once



#if defined(__linux__)

#include <qinmo/net/ReactorTcpServer.h>
#include <qinmo/net/EventLoop.h>
namespace qinmo::net
{
using TcpConnection = ReactorTcpConnection;
using TcpServer = ReactorTcpServer;
} // namespace qinmo::net

#elif defined(_WIN32)

#include <qinmo/net/TcpListen.h>
#include <qinmo/net/SocketUDP.h>

#else

#error "Platform not supported"

#endif


#include <qinmo/base/Logger.h>
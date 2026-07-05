/**
 * @file <qinmo/net.h>
 * @brief A Simple Net Library
 * @details This module covers:
 *      business-layer           : ReactorTCPServer & ReactorTCPConnection
 *      Low-level                : SocketTCP & SocketUDP
 *      Common non-net utilities : println, StringView, Logger ...
 * 
 * @version 2.2.10
 * @author qinmoM
 * @github https://github.com/qinmoM/qinmo_net
 */
#pragma once


#include <qinmo/base/Logger.h>


#if defined(__linux__)

#include <qinmo/net/ReactorTcpServer.h>
#include <qinmo/net/EventLoop.h>
namespace qinmo
{
namespace net
{
using TcpConnection = ReactorTcpConnection;
using TcpServer = ReactorTcpServer;
} // namespace qinmo::net
} // namespace qinmo

#elif defined(_WIN32)

#error "Platform not supported"

#else

#error "Platform not supported"

#endif

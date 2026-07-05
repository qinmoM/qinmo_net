#pragma once

#include "TcpConnect.h"
#include "EventLoopThreadPool.h"
#include "PacketBuffer.h"
#include "Channel.h"
#include "TimerID.h"
#include <set>

/// @namespace qinmo
namespace qinmo
{
/// @namespace qinmp::net
namespace net
{

class ReactorTcpConnection;



using RTcpConnPtr = std::shared_ptr<ReactorTcpConnection>;

/// @brief connecting callback
using TimerConnFunc = std::function<void(const RTcpConnPtr&)>;
/// @brief connecting callback
using ConnectFunc = std::function<void(const RTcpConnPtr&)>;
/// @brief disconnection callback
using DisconnectFunc = std::function<void(const RTcpConnPtr&)>;
/// @brief send complete callback
using WriteCompleteFunc = std::function<void(const RTcpConnPtr&)>;
/// @brief new message callback
using MessageFunc = std::function<void(const RTcpConnPtr&, PacketBuffer&, Timestamp)>;
/// @brief send buffer full callback
using HighWaterMarkFunc = std::function<void(const RTcpConnPtr&, std::size_t)>;

/// @brief close callback
/// @warning for intarnal use only, do NOT use it from outside the library
using CloseFunc = std::function<void()>;



class ReactorTcpConnection
    // Inherit this template class  when  needing to create shared_ptr via 'this' pointer inside the class
    : public std::enable_shared_from_this<ReactorTcpConnection>
{
public:
    ReactorTcpConnection(EventLoop* loop, TcpConnect&& sock, const InetAddr& localAddr, const InetAddr& peerAddr);
    ~ReactorTcpConnection();

    ReactorTcpConnection(const ReactorTcpConnection&) = delete;
    ReactorTcpConnection& operator=(const ReactorTcpConnection&) = delete;

    ReactorTcpConnection(ReactorTcpConnection&&) = delete;
    ReactorTcpConnection& operator=(ReactorTcpConnection&&) = delete;

public:
    int getfd() const;
    bool getIsConnect() const;
    EventLoop* getLoop() const;
    InetAddr getLocalAddr() const;
    InetAddr getPeerAddr() const;

    void connectEstablished();
    void connectDestroyed();

    TimerID timerAt(Timestamp timestamp, TimerConnFunc func);
    TimerID timerAfter(double seconds, TimerConnFunc func);
    TimerID timerRepeatAt(Timestamp timestamp, double intervalSeconds, TimerConnFunc func);
    TimerID timerRepeatAfter(double beginSeconds, double intervalSeconds, TimerConnFunc func);
    void timerCancel(TimerID id);

    void send(const std::string& str);
    void send(PacketBuffer& buf);

    void setTcpNoDelay(bool enable);
    /// @note do nothing when in write event
    void shutdown();
    void close();

    void setConnectFunc(const ConnectFunc& f);
    void setDisconnectFunc(const DisconnectFunc& f);
    void setWriteCompleteFunc(const WriteCompleteFunc& f);
    void setMessageFunc(const MessageFunc& f);
    void setHighWaterMarkFunc(std::size_t waterMark, const HighWaterMarkFunc& f);
    /// @warning internal use only
    void setCloseFunc(const CloseFunc& f);

private:
    enum class RTcpConnState : char
    {
        Disconnected = 0,
        Connecting,
        Connected,
        Disconnecting
    };
    qinmo::StringView getStateStr() const;

    void handleRead(Timestamp time);
    void handleWrite();
    void handleClose();
    void handleError();

private:
    EventLoop* loop_;
    RTcpConnState state_;

    TcpConnect sock_;
    Channel channel_;
    InetAddr localAddr_;
    InetAddr peerAddr_;

    ConnectFunc connectFunc_;
    DisconnectFunc disconnectFunc_;
    MessageFunc messageFunc_;
    WriteCompleteFunc writeCompleteFunc_;
    HighWaterMarkFunc highWaterMarkFunc_;
    CloseFunc closeFunc_;
    std::size_t waterMark_;

    PacketBuffer inputBuffer_;
    PacketBuffer outputBuffer_;

    std::set<TimerID> timers_;

};

;
} // namespace net
} // namespace qinmo
#include <qinmo/net.h>

int main()
{
    using namespace qinmo;
    using namespace qinmo::net;

    EventLoop loop;

    InetAddr addr(true);
    addr.setIP("192.168.233.90");
    addr.setPort(7129);
    TcpServer server(&loop, addr, 1);
    server.setConnectFunc(
        [](const RTcpConnPtr& conn) ->void
        {
            InetAddr peer = conn->getPeerAddr();
            println("new connect. ip:", peer.getIP(), ", port:", peer.getPort());

            TimerID id = conn->timerRepeatAfter(
                5.0,
                5.0,
                [](const RTcpConnPtr& conn) -> void
                {
                    if (conn->getIsConnect())
                        println("5 seconds up.");
                }
            );
        }
    );
    server.setDisconnectFunc(
        [](const RTcpConnPtr& conn) ->void
        {
            InetAddr peer = conn->getPeerAddr();
            qinmo::println("disconnect. ip:", peer.getIP(), ", port:", peer.getPort());
        }
    );

    server.start();
    loop.loop();

    return 0;
}
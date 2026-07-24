#include <qinmo/net.h>

int main()
{
    qinmo::net::EventLoop loop;

    qinmo::net::InetAddr addr;
    addr.setIP("192.168.233.90");
    addr.setPort(7129);

    qinmo::net::TcpServer server(&loop, addr, 0);
    server.setConnectFunc(
        [](const qinmo::net::RTcpConnPtr& conn) -> void
        {
            qinmo::println(conn->getPeerAddr().getIP(), ":", conn->getPeerAddr().getPort(), " connect.");
        }
    );
    server.setDisconnectFunc(
        [](const qinmo::net::RTcpConnPtr& conn) -> void
        {
            qinmo::println(conn->getPeerAddr().getIP(), ":", conn->getPeerAddr().getPort(), " disconnect.");
        }
    );

    server.start();
    loop.loop();

    return 0;
}
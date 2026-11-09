#include <qinmo/net.h>

int main()
{
    using namespace qinmo;
    using namespace qinmo::net;

    EventLoop loop;

    InetAddr seraddr(true);
    seraddr.setIP("192.168.252.212");
    seraddr.setPort(7129);

    TcpClient client(&loop, seraddr);
    client.setRetry(true);
    client.setMessageFunc(
        [](const RTcpConnPtr& conn, PacketBuffer& input, Timestamp time) -> void
        {
            std::string str;
            input.retrieveAll(str);
            println(str);
            conn->send(str);
        }
    );

    client.connect();
    loop.loop();

    return 0;
}
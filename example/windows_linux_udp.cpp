#include <qinmo/net.h>

int main()
{
    using namespace qinmo;
    using namespace qinmo::net;

    InetAddr local;
    local.setIP("127.0.0.1");
    local.setPort(7128);

    InetAddr peer;
    peer.setIP("192.168.167.90");
    peer.setPort(7129);

    SocketUDP sock = SocketUDP::createRaw(local);
    if (sock.isValid())
        std::cout << "OK" << std::endl;

    std::cout << sock.sendto("hello", 6, peer) << std::endl;

    return 0;
}
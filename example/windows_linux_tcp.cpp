#include <qinmo/net.h>

int main()
{
    using namespace qinmo;
    using namespace qinmo::net;
    auto exitError = [](bool error, std::string s) -> void
    {
        if (error) return;

        std::cout << "Failed to " << s << "." << std::endl;
        std::exit(-1);
    };

    InetAddr local;
    local.setIP("127.0.0.1");
    local.setPort(7128);

    InetAddr peer;
    peer.setIP("192.168.167.90");
    peer.setPort(7129);

    SocketTCP sock = SocketTCP::createRaw(local);
    exitError(!sock.isValid(), "create");
    exitError(!sock.connect(peer), "connect");

    std::cout << sock.send("hello\n", 7) << std::endl;

    return 0;
}
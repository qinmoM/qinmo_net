/**
 * @note Run inside a virtual machine, without parsing HTTP and maybe even go through the physical network card. So the data is not reliable.

 ./wrk -t4 -c1000 -d30s http://127.0.0.1:7129/

Running 30s test @ http://127.0.0.1:7129/
  4 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     2.85ms    4.01ms 211.56ms   92.84%
    Req/Sec    87.99k     7.47k  210.51k    82.02%
  10515589 requests in 30.10s, 641.82MB read
Requests/sec: 349329.20
Transfer/sec:     21.32MB

 * @note The change in file descriptors before and after test:
before:
ls /proc/51711/fd
0  1  10  11  12  13  14  15  16  17  18  2  3  4  5  6  7  8  9

after:
ls /proc/51711/fd
0  1  10  106  11  12  13  14  15  16  17  18  2  3  4  5  6  7  8  9

 */

#include <qinmo/net.h>

int main()
{
    using namespace qinmo;
    using namespace qinmo::net;

    EventLoop loop;

    InetAddr addr(true);
    addr.setIP("127.0.0.1");
    addr.setPort(7129);
    TcpServer server(&loop, addr, 4);
    server.setMessageFunc(
        [](const RTcpConnPtr& conn, PacketBuffer& buffer, Timestamp time) -> void
        {
            std::string res;
            buffer.retrieveAll(res);

            res = "HTTP/1.1 200 OK\r\n"
                  "Content-Length: 2\r\n"
                  "Connection: keep-alive\r\n"
                  "\r\n"
                  "OK";
            conn->send(res);
        }
    );

    server.start();
    loop.loop();

    return 0;
}
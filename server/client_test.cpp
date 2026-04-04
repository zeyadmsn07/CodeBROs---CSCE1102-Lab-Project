#include <boost/asio.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

int main()
{
    try
    {
        boost::asio::io_context io_ctx;

        tcp::resolver resolver(io_ctx);
        auto endpoints = resolver.resolve("127.0.0.1", "12345");

        tcp::socket socket(io_ctx);
        boost::asio::connect(socket, endpoints);

        std::cout << "[client] connected to server\n";

        std::string message = "HELLO\n";
        boost::asio::write(socket,
                           boost::asio::buffer(message));

        std::cout << "[client] sent: HELLO\n";

        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, '\n');

        std::istream stream(&response);
        std::string reply;
        std::getline(stream, reply);

        std::cout << "[client] received: " << reply << "\n";

        socket.close();
        std::cout << "[client] disconnected\n";
    }
    catch (std::exception& e)
    {
        std::cerr << "[client] error: " << e.what() << "\n";
    }

    return 0;
}

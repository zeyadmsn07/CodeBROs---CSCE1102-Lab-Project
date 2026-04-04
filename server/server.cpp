#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>

using boost::asio::ip::tcp;
class Session : public std::enable_shared_from_this<Session>
{
public:
    explicit Session(tcp::socket socket)
        : socket_(std::move(socket))
    {}

    void start()
    {
        std::cout << "[server] client connected: "
                  << socket_.remote_endpoint()
                  << "\n";
        do_read();
    }

private:
    void do_read()
    {
        auto self = shared_from_this();

        boost::asio::async_read_until(
            socket_,
            read_buf_,
            '\n',
            [self](boost::system::error_code ec,
                   std::size_t bytes_transferred)
            {
                if (ec)
                {
                    std::cout << "[server] client disconnected\n";
                    return;
                }

                std::istream stream(&self->read_buf_);
                std::string line;
                std::getline(stream, line);

                std::cout << "[server] received: " << line << "\n";

                self->do_write(line + "\n");
            });
    }

    void do_write(const std::string& message)
    {
        auto self = shared_from_this();

        auto buf = std::make_shared<std::string>(message);

        boost::asio::async_write(
            socket_,
            boost::asio::buffer(*buf),
            [self, buf](boost::system::error_code ec,
                        std::size_t)
            {
                if (!ec)
                {
                    self->do_read();
                }
            });
    }

    tcp::socket                 socket_;
    boost::asio::streambuf      read_buf_;
};

class Server
{
public:
    Server(boost::asio::io_context& io_ctx, unsigned short port)
        : acceptor_(io_ctx,
                    tcp::endpoint(tcp::v4(), port))
    {
        std::cout << "[server] listening on port "
                  << port << "\n";
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec,
                   tcp::socket socket)
            {
                if (!ec)
                {
                    std::make_shared<Session>(
                        std::move(socket))->start();
                }
                do_accept();
            });
    }

    tcp::acceptor acceptor_;
};

int main()
{
    try
    {
        boost::asio::io_context io_ctx;
        Server server(io_ctx, 12345);
        io_ctx.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "[server] error: " << e.what() << "\n";
    }

    return 0;
}

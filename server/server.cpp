#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <deque>
#include <mutex>
#include <algorithm>
#include "MessageTypes.h"
#include "MessageFactory.h"
#include "Message.h"
using boost::asio::ip::tcp;
using json = nlohmann::json;

class Server;
class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(tcp::socket socket, Server& server)
        : socket_(std::move(socket)), server_(server) {}

    ~Session();

    void start()
    {
        std::cout << "[server] client connected: "
                  << socket_.remote_endpoint() << "\n";
        read_next();
    }
    void send(const std::string& msg)
    {
        write_queue_.push_back(msg);
        if (!writing_) do_write();
    }

    std::string username() const { return username_; }
    std::string partyId()  const { return partyId_;  }

private:
    void read_next()
    {
        auto self = shared_from_this();
        boost::asio::async_read_until(socket_, buf_, '\n',
            [self](boost::system::error_code ec, std::size_t)
            {
                if (ec) {
                    std::cout << "[server] client disconnected ("
                              << self->username_ << ")\n";
                    return;
                }
                std::istream stream(&self->buf_);
                std::string line;
                std::getline(stream, line);

                self->dispatch(line);
                self->read_next();
            });
    }

    void do_write()
    {
        if (write_queue_.empty()) { writing_ = false; return; }
        writing_ = true;
        auto self = shared_from_this();
        auto data = std::make_shared<std::string>(write_queue_.front());
        write_queue_.pop_front();

        boost::asio::async_write(socket_, boost::asio::buffer(*data),
            [self, data](boost::system::error_code ec, std::size_t)
            {
                if (!ec) self->do_write();
            });
    }
    void dispatch(const std::string& line);
    void handle_join      (const Message& msg);
    void handle_chat      (const Message& msg);
    void handle_leave     (const Message& msg);
    void handle_code_sync (const Message& msg);
    tcp::socket             socket_;
    boost::asio::streambuf  buf_;
    Server&                 server_;

    std::string             username_;
    std::string             partyId_;

    std::deque<std::string> write_queue_;
    bool                    writing_ = false;
};
class Server
{
public:
    Server(boost::asio::io_context& io, unsigned short port)
        : acceptor_(io, tcp::endpoint(tcp::v4(), port))
    {
        std::cout << "[server] listening on port " << port << "\n";
        accept_next();
    }

    void addToRoom(const std::string& partyId,
                   std::shared_ptr<Session> session)
    {
        std::lock_guard<std::mutex> lock(rooms_mutex_);
        rooms_[partyId].push_back(session);
        std::cout << "[server] room " << partyId
                  << " now has " << rooms_[partyId].size() << " member(s)\n";
    }
    void removeFromAllRooms(Session* raw)
    {
        std::lock_guard<std::mutex> lock(rooms_mutex_);
        for (auto& [partyId, sessions] : rooms_) {
            sessions.erase(
                std::remove_if(sessions.begin(), sessions.end(),
                    [raw](const std::weak_ptr<Session>& wp) {
                        auto sp = wp.lock();
                        return !sp || sp.get() == raw;   // purge dead OR this session
                    }),
                sessions.end());
        }
        for (auto it = rooms_.begin(); it != rooms_.end(); ) {
            it = it->second.empty() ? rooms_.erase(it) : std::next(it);
        }
    }
    void broadcast(const std::string& partyId, const json& msg, Session* exclude)
    {
        std::string serialized = msg.dump() + "\n";
        std::lock_guard<std::mutex> lock(rooms_mutex_);

        auto it = rooms_.find(partyId);
        if (it == rooms_.end()) return;

        auto& sessions = it->second;
        std::vector<std::weak_ptr<Session>> alive;

        for (auto& wp : sessions) {
            auto sp = wp.lock();
            if (!sp) continue;           // dead → drop from room (purge)
            alive.push_back(wp);         
            if (sp.get() != exclude)
                sp->send(serialized);    
        }
        sessions = std::move(alive);     
    }

    // Sends updated MEMBER_LIST to every client in the room (including sender).
    // Used after join, leave, and abrupt disconnect.
    void broadcastMemberList(const std::string& partyId)
    {
        std::lock_guard<std::mutex> lock(rooms_mutex_);

        auto it = rooms_.find(partyId);
        if (it == rooms_.end()) return;  // room is empty / already deleted

        // Collect current usernames
        std::vector<std::string> names;
        for (auto& wp : it->second) {
            if (auto sp = wp.lock())
                names.push_back(sp->username());
        }

        std::string serialized =
            MessageFactory::toJsonString(
                MessageFactory::buildMemberList(partyId, names)) + "\n";

        // Send to everyone in the room (including the newly joined client)
        for (auto& wp : it->second) {
            if (auto sp = wp.lock())
                sp->send(serialized);
        }
    }

private:
    void accept_next()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                    std::make_shared<Session>(std::move(socket), *this)->start();
                accept_next();
            });
    }

    tcp::acceptor acceptor_;
    std::map<std::string,
             std::vector<std::weak_ptr<Session>>> rooms_;
    std::mutex rooms_mutex_;
};

Session::~Session()
{
    if (!partyId_.empty()) {
        std::cout << "[server] cleaning up " << username_
                  << " from room " << partyId_ << "\n";
        server_.removeFromAllRooms(this);
        server_.broadcastMemberList(partyId_);
    }
}

void Session::dispatch(const std::string& line)
{
    Message msg = MessageFactory::fromJsonString(line);

    if      (msg.type == MessageTypes::JOIN)      handle_join(msg);
    else if (msg.type == MessageTypes::LEAVE)     handle_leave(msg);
    else if (msg.type == MessageTypes::CHAT)      handle_chat(msg);
    else if (msg.type == MessageTypes::CODE_SYNC) handle_code_sync(msg);
    else
        std::cout << "[server] unhandled type: " << msg.type << "\n";
}

void Session::handle_join(const Message& msg)
{
    username_ = msg.sender;
    partyId_  = msg.partyId;

    server_.addToRoom(partyId_, shared_from_this());

    std::cout << "[server] JOIN  " << username_
              << " → room " << partyId_ << "\n";

    // Tell everyone in the room (including the joiner) about the new list
    server_.broadcastMemberList(partyId_);
}

void Session::handle_chat(const Message& msg)
{
    std::cout << "[server] CHAT  [" << msg.partyId << "] "
              << msg.sender << ": " << msg.payload << "\n";

    // Forward to everyone in the room EXCEPT the sender
    server_.broadcast(msg.partyId, MessageFactory::toJson(msg), this);
}

void Session::handle_leave(const Message& msg)
{
    std::cout << "[server] LEAVE " << msg.sender
              << " ← room " << msg.partyId << "\n";

    std::string oldParty = partyId_;   // save before clearing
    server_.removeFromAllRooms(this);
    partyId_  = "";
    username_ = "";

    server_.broadcastMemberList(oldParty);  // notify remaining members
}

void Session::handle_code_sync(const Message& msg)
{
    // Relay code to everyone in room except sender — no validation
    server_.broadcast(msg.partyId, MessageFactory::toJson(msg), this);
}

int main()
{
    try {
        boost::asio::io_context io;
        Server server(io, 12345);
        io.run();
    }
    catch (std::exception& e) {
        std::cerr << "[server] fatal: " << e.what() << "\n";
    }
}

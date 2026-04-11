#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <algorithm>
#include "RoomManager.h"

using boost::asio::ip::tcp;
using json = nlohmann::json;

class Server;

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(tcp::socket socket, Server& server, RoomManager& rm)
        : socket_(std::move(socket)), server_(server), rooms_(rm)
    {}

    void start() { read_next(); }

    void send(const std::string& msg)
    {
        auto self = shared_from_this();
        auto data = std::make_shared<std::string>(msg + "\n");
        boost::asio::async_write(socket_, boost::asio::buffer(*data),
            [self, data](boost::system::error_code, std::size_t){});
    }

    std::string username;

private:
    void read_next();
    void handle(const json& msg);
    void broadcastToRoom(const std::string& roomId, const std::string& data, Session* exclude);

    tcp::socket    socket_;
    boost::asio::streambuf buf_;
    Server&        server_;
    RoomManager&   rooms_;
    std::string    current_room;
};

// ── Server ────────────────────────────────────

class Server
{
public:
    Server(boost::asio::io_context& io, unsigned short port)
        : acceptor_(io, tcp::endpoint(tcp::v4(), port))
    {
        std::cout << "listening on port " << port << "\n";
        accept_next();
    }

private:
    void accept_next()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                    std::make_shared<Session>(
                        std::move(socket), *this, roomMgr)->start();
                accept_next();
            });
    }

    tcp::acceptor acceptor_;
    RoomManager   roomMgr;
};

// ── Session implementation ────────────────────

void Session::broadcastToRoom(const std::string& roomId,
                               const std::string& data,
                               Session* exclude)
{
    Room* r = rooms_.getRoom(roomId);
    if (!r) return;
    for (auto& wp : r->members) {
        auto sp = wp.lock();
        if (!sp || sp.get() == exclude) continue;
        sp->send(data);
    }
}

void Session::read_next()
{
    auto self = shared_from_this();
    boost::asio::async_read_until(socket_, buf_, '\n',
        [self](boost::system::error_code ec, std::size_t)
        {
            if (ec) {
                std::cout << "client disconnected: " << self->username << "\n";
                // notify room if user was in one
                if (!self->current_room.empty()) {
                    std::lock_guard<std::mutex> lk(self->rooms_.mtx);
                    self->rooms_.removeMember(self->current_room, self);
                    self->broadcastToRoom(self->current_room, json({
                        {"type", "user_left"}, {"username", self->username}
                    }).dump(), nullptr);
                }
                return;
            }

            std::istream stream(&self->buf_);
            std::string  line;
            std::getline(stream, line);

            try {
                self->handle(json::parse(line));
            }
            catch (...) {
                std::cout << "bad json\n";
            }

            self->read_next();
        });
}

void Session::handle(const json& msg)
{
    std::string type = msg.value("type", "");

    // ---- get_rooms ----
    if (type == "get_rooms") {
        std::lock_guard<std::mutex> lk(rooms_.mtx);
        auto list = rooms_.listRooms();
        json resp;
        resp["type"]  = "room_list";
        resp["rooms"] = json::array();
        for (auto& [id, name, cnt] : list)
            resp["rooms"].push_back({{"id", id}, {"name", name}, {"members", cnt}});
        send(resp.dump());
        return;
    }

    // ---- create_room ----
    if (type == "create_room") {
        std::string rname = msg.value("name", "Unnamed Room");
        std::lock_guard<std::mutex> lk(rooms_.mtx);
        std::string rid = rooms_.createRoom(rname);
        rooms_.addMember(rid, shared_from_this());
        current_room = rid;
        send(json({{"type", "room_created"}, {"room_id", rid}, {"name", rname}}).dump());
        return;
    }

    // ---- join_room ----
    if (type == "join_room") {
        std::string rid = msg.value("room_id", "");
        std::lock_guard<std::mutex> lk(rooms_.mtx);
        Room* r = rooms_.getRoom(rid);
        if (!r) {
            send(json({{"type", "join_failed"}, {"reason", "room not found"}}).dump());
            return;
        }
        current_room = rid;
        rooms_.addMember(rid, shared_from_this());
        // send current code state to the new joiner
        send(json({{"type", "room_joined"}, {"room_id", rid}, {"code", r->current_code}}).dump());
        // tell everyone else
        broadcastToRoom(rid, json({{"type", "user_joined"}, {"username", username}}).dump(), this);
        return;
    }

    // ---- leave_room ----
    if (type == "leave_room") {
        if (!current_room.empty()) {
            std::lock_guard<std::mutex> lk(rooms_.mtx);
            rooms_.removeMember(current_room, shared_from_this());
            broadcastToRoom(current_room, json({
                {"type", "user_left"}, {"username", username}
            }).dump(), this);
            current_room.clear();
        }
        return;
    }

    // ---- code_update ----
    if (type == "code_update") {
        if (current_room.empty()) return;
        std::lock_guard<std::mutex> lk(rooms_.mtx);
        Room* r = rooms_.getRoom(current_room);
        if (!r) return;
        r->current_code = msg.value("code", "");
        broadcastToRoom(current_room, msg.dump(), this); // don't echo back to sender
        return;
    }

    // ---- chat_message ----
    if (type == "chat_message") {
        if (current_room.empty()) return;
        broadcastToRoom(current_room, msg.dump(), nullptr); // everyone including sender
        return;
    }

    // ---- login (store username) ----
    if (type == "login") {
        username = msg.value("username", "");
        // existing login logic stays here unchanged
        return;
    }
}

// ── main ─────────────────────────────────────

int main()
{
    try {
        boost::asio::io_context io;
        Server server(io, 12345);
        io.run();
    }
    catch (std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
    }
}
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "RoomManager.h"
#include "UserStore.h"

using boost::asio::ip::tcp;
using json = nlohmann::json;

class Server;

struct Session : public std::enable_shared_from_this<Session> {
    Session(tcp::socket socket, Server& server, RoomManager& rooms, UserStore& userStore)
        : socket_(std::move(socket)),
          server_(server),
          rooms_(rooms),
          userStore_(userStore) {}

    void start() { read_next(); }

    void send(const std::string& msg) {
        auto self = shared_from_this();
        auto data = std::make_shared<std::string>(msg + "\n");
        boost::asio::async_write(socket_, boost::asio::buffer(*data),
                                 [self, data](boost::system::error_code, std::size_t) {});
    }

    std::string username;

private:
    void read_next();
    void handle(const json& msg);
    void handleDisconnect();
    void broadcastToRoom(const std::string& roomId, const std::string& data, Session* exclude);
    void broadcastMemberList(const std::string& roomId);

    tcp::socket            socket_;
    boost::asio::streambuf buf_;
    Server&                server_;
    RoomManager&           rooms_;
    UserStore&             userStore_;
    std::string            current_room;
};

class Server {
public:
    Server(boost::asio::io_context& io, unsigned short port)
        : acceptor_(io, tcp::endpoint(tcp::v4(), port)),
          userStore_("data/users.json") {
        std::cout << "listening on port " << port << "\n";
        accept_next();
    }

private:
    void accept_next() {
        acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec)
                std::make_shared<Session>(
                    std::move(socket), *this, roomMgr_, userStore_)->start();
            accept_next();
        });
    }

    tcp::acceptor acceptor_;
    RoomManager   roomMgr_;
    UserStore     userStore_;
};

void Session::broadcastToRoom(const std::string& roomId,
                              const std::string& data,
                              Session* exclude) {
    Room* r = rooms_.getRoom(roomId);
    if (!r) return;
    for (auto& wp : r->members) {
        auto sp = wp.lock();
        if (!sp || sp.get() == exclude) continue;
        sp->send(data);
    }
}

// sends the current member list to everyone in the room
void Session::broadcastMemberList(const std::string& roomId) {
    Room* r = rooms_.getRoom(roomId);
    if (!r) return;

    json names = json::array();
    for (auto& wp : r->members) {
        auto sp = wp.lock();
        if (sp && !sp->username.empty())
            names.push_back(sp->username);
    }

    std::string msg = json({{"type", "member_list"}, {"members", names}}).dump();
    broadcastToRoom(roomId, msg, nullptr); // send to everyone including self
}

void Session::handleDisconnect() {
    std::cout << "client disconnected: " << username << "\n";
    if (!current_room.empty()) {
        std::lock_guard<std::mutex> lk(rooms_.mtx);
        rooms_.removeMember(current_room, shared_from_this());
        broadcastToRoom(current_room,
            json({{"type", "user_left"}, {"username", username}}).dump(),
            nullptr);
        broadcastMemberList(current_room);
        current_room.clear();
    }
}

void Session::read_next() {
    auto self = shared_from_this();
    boost::asio::async_read_until(socket_, buf_, '\n',
        [self](boost::system::error_code ec, std::size_t) {
            if (ec) {
                self->handleDisconnect();
                return;
            }

            std::istream stream(&self->buf_);
            std::string  line;
            std::getline(stream, line);

            try {
                self->handle(json::parse(line));
            } catch (...) {
                std::cout << "bad json: " << line << "\n";
            }

            self->read_next();
        });
}

void Session::handle(const json& msg) {
    std::string type = msg.value("type", "");

    if (type == "session_restore") {
        username = msg.value("username", "");
        return;
    }

    if (type == "register") {
        std::string uname = msg.value("username", "");
        std::string pass  = msg.value("password", "");

        if (uname.empty() || pass.empty()) {
            send(json({{"type", "register_failed"},
                       {"reason", "missing fields"}}).dump());
            return;
        }

        bool ok = userStore_.registerUser(uname, pass);
        if (ok)
            send(json({{"type", "register_success"}}).dump());
        else
            send(json({{"type", "register_failed"},
                       {"reason", "username already taken"}}).dump());
        return;
    }

    if (type == "login") {
        std::string uname = msg.value("username", "");
        std::string pass  = msg.value("password", "");

        if (uname.empty()) {
            send(json({{"type", "login_failed"},
                       {"reason", "missing username"}}).dump());
            return;
        }

        // session restore path — no password needed
        if (pass.empty()) {
            username = uname;
            send(json({{"type", "login_success"}, {"username", uname}}).dump());
            return;
        }

        bool ok = userStore_.authenticate(uname, pass);
        if (ok) {
            username = uname;
            userStore_.updateLastLogin(uname);
            send(json({{"type", "login_success"}, {"username", uname}}).dump());
        } else {
            send(json({{"type", "login_failed"},
                       {"reason", "invalid username or password"}}).dump());
        }
        return;
    }

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

    if (type == "create_room") {
        // prevent joining if already in a room
        if (!current_room.empty()) {
            send(json({{"type", "join_failed"},
                       {"reason", "You are already in a room. Log out to leave."}}).dump());
            return;
        }

        std::string rname = msg.value("name", "Unnamed Room");
        std::lock_guard<std::mutex> lk(rooms_.mtx);

        // global check: prevent joining if already in ANY room with same username
        if (!username.empty()) {
            for (auto& wp : rooms_.getAllMembers()) {
                auto sp = wp.lock();
                if (sp && sp->username == username) {
                    send(json({{"type", "join_failed"},
                               {"reason", "You are already in a room."}}).dump());
                    return;
                }
            }
        }

        std::string rid = rooms_.createRoom(rname);
        rooms_.addMember(rid, shared_from_this());
        current_room = rid;
        send(json({{"type", "room_created"},
                   {"room_id", rid},
                   {"name", rname}}).dump());
        broadcastMemberList(rid);
        return;
    }

    if (type == "join_room") {
        // prevent joining if already in a room
        if (!current_room.empty()) {
            send(json({{"type", "join_failed"},
                       {"reason", "You are already in a room. Log out to leave."}}).dump());
            return;
        }

        std::string rid = msg.value("room_id", "");
        std::lock_guard<std::mutex> lk(rooms_.mtx);
        Room* r = rooms_.getRoom(rid);
        if (!r) {
            send(json({{"type", "join_failed"},
                       {"reason", "room not found"}}).dump());
            return;
        }

        // global check: prevent joining if already in ANY room with same username
        if (!username.empty()) {
            for (auto& wp : rooms_.getAllMembers()) {
                auto sp = wp.lock();
                if (sp && sp->username == username) {
                    send(json({{"type", "join_failed"},
                               {"reason", "You are already in a room."}}).dump());
                    return;
                }
            }
        }

        current_room = rid;
        rooms_.addMember(rid, shared_from_this());
        send(json({{"type",    "room_joined"},
                   {"room_id", rid},
                   {"name",    r->name},
                   {"code",    r->current_code}}).dump());
        broadcastToRoom(rid,
            json({{"type", "user_joined"}, {"username", username}}).dump(),
            this);
        broadcastMemberList(rid);
        return;
    }

    if (type == "leave_room") {
        if (!current_room.empty()) {
            std::lock_guard<std::mutex> lk(rooms_.mtx);
            rooms_.removeMember(current_room, shared_from_this());
            broadcastToRoom(current_room,
                json({{"type", "user_left"}, {"username", username}}).dump(),
                this);
            broadcastMemberList(current_room);
            current_room.clear();
        }
        return;
    }

    if (type == "code_update") {
        if (current_room.empty()) return;
        std::lock_guard<std::mutex> lk(rooms_.mtx);
        Room* r = rooms_.getRoom(current_room);
        if (!r) return;
        r->current_code = msg.value("code", "");
        broadcastToRoom(current_room, msg.dump(), this);
        return;
    }

    if (type == "chat_message") {
        if (current_room.empty()) return;
        broadcastToRoom(current_room, msg.dump(), nullptr);
        return;
    }
}

int main() {
    try {
        boost::asio::io_context io;
        Server server(io, 12345);
        io.run();
    } catch (std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
    }
}
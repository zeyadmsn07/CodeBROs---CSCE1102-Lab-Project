#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>

// forward declare to avoid including Session.h here
struct Session;

struct Room {
    std::string id;
    std::string name;
    std::string current_code;
    std::vector<std::weak_ptr<Session>> members;
};

class RoomManager {
public:
    // creates a new room, returns its id
    std::string createRoom(const std::string& name);

    // returns nullptr if room doesn't exist
    Room* getRoom(const std::string& roomId);

    // returns list of {id, name, member count}
    std::vector<std::tuple<std::string, std::string, int>> listRooms();

    void addMember(const std::string& roomId, std::shared_ptr<Session> s);
    void removeMember(const std::string& roomId, std::shared_ptr<Session> s);

    std::mutex mtx; // lock this before touching rooms

private:
    std::map<std::string, Room> rooms;
    int nextId = 1;
};
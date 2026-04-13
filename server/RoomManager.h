#pragma once
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

struct Session;

struct Room {
    std::string id;
    std::string name;
    std::string current_code;
    std::vector<std::weak_ptr<Session>> members;
};

class RoomManager {
   public:
    std::string createRoom(const std::string& name);  // creates a new room, returns its id
    Room* getRoom(const std::string& roomId);         // returns nullptr if room doesn't exist
    std::vector<std::tuple<std::string, std::string, int>>
    listRooms();  // returns list of {id, name, member count}
    void addMember(const std::string& roomId, std::shared_ptr<Session> s);
    void removeMember(const std::string& roomId, std::shared_ptr<Session> s);
    std::mutex mtx;

   private:
    std::map<std::string, Room> rooms;
    int nextId = 1;
};
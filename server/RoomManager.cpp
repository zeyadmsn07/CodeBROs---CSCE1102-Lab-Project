#include "RoomManager.h"
#include <algorithm>

std::string RoomManager::createRoom(const std::string& name) {
    std::string id = "room_" + std::to_string(nextId++);
    rooms[id] = Room{ id, name, "", {} };
    return id;
}

Room* RoomManager::getRoom(const std::string& roomId) {
    auto it = rooms.find(roomId);
    if (it == rooms.end()) return nullptr;
    return &it->second;
}

std::vector<std::tuple<std::string, std::string, int>> RoomManager::listRooms() {
    std::vector<std::tuple<std::string, std::string, int>> out;
    for (auto& [id, r] : rooms) {
        int alive = 0;
        for (auto& wp : r.members)
            if (!wp.expired()) alive++;
        out.push_back({ id, r.name, alive });
    }
    return out;
}

void RoomManager::addMember(const std::string& roomId, std::shared_ptr<Session> s) {
    auto* r = getRoom(roomId);
    if (!r) return;
    r->members.push_back(s);
}

void RoomManager::removeMember(const std::string& roomId, std::shared_ptr<Session> s) {
    auto* r = getRoom(roomId);
    if (!r) return;
    // remove expired pointers and the given session
    r->members.erase(
        std::remove_if(r->members.begin(), r->members.end(),
            [&](std::weak_ptr<Session>& wp) {
                auto sp = wp.lock();
                return !sp || sp.get() == s.get();
            }),
        r->members.end()
    );
}
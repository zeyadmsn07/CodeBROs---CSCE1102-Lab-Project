#ifndef MESSAGEFACTORY_H
#define MESSAGEFACTORY_H
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "Message.h"
#include "MessageTypes.h"

class MessageFactory {
   public:
    static Message buildJoin(const std::string& username, const std::string& partyId);
    static Message buildLeave(const std::string& username, const std::string& partyId);
    static Message buildChat(const std::string& sender, const std::string& partyId,
                             const std::string& text);
    static Message buildMemberList(const std::string& partyId,
                                   const std::vector<std::string>& members);
    static Message buildCodeSync(const std::string& sender, const std::string& partyId,
                                 const std::string& code);
    static Message buildTaskSubmit(const std::string& sender, const std::string& partyId,
                                   int taskId, const std::string& answer);
    static Message buildTaskResult(const std::string& partyId, int taskId, bool passed,
                                   const std::string& feedback);
    static Message buildPing(const std::string& partyId);
    static Message buildPong(const std::string& partyId);
    static Message buildAuthOk(const std::string& username);
    static Message buildAuthFail(const std::string& reason);
    static Message buildError(const std::string& partyId, const std::string& reason);
    static nlohmann::json toJson(const Message& msg);
    static std::string toJsonString(const Message& msg);
    static Message fromJson(const nlohmann::json& j);
    static Message fromJsonString(const std::string& raw);
    // room + editor methods (task 17)
    static nlohmann::json makeLogin(const std::string& username, const std::string& password);
    static nlohmann::json makeRegister(const std::string& username, const std::string& password);
    static nlohmann::json makeGetRooms();
    static nlohmann::json makeCreateRoom(const std::string& roomName);
    static nlohmann::json makeJoinRoom(const std::string& roomId);
    static nlohmann::json makeLeaveRoom();
    static nlohmann::json makeCodeUpdate(const std::string& roomId, const std::string& code);
    static nlohmann::json makeChatMsg(const std::string& roomId, const std::string& sender,
                                      const std::string& text);

    static nlohmann::json makeSessionRestore(const std::string& username);
};

#endif
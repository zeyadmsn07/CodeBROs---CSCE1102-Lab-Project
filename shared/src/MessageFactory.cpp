#include "MessageFactory.h"
#include <ctime>
#include <stdexcept>
static long now()
{
    return static_cast<long>(std::time(nullptr));
}

static Message make(const std::string& type, const std::string& sender, const std::string& partyId, const std::string& payload)
{return Message(type, sender, partyId, payload, now());}

Message MessageFactory::buildJoin(const std::string& username, const std::string& partyId)
{
    return make(MessageTypes::JOIN, username, partyId, "");
}

Message MessageFactory::buildLeave(const std::string& username, const std::string& partyId)
{
    return make(MessageTypes::LEAVE, username, partyId, "");
}

Message MessageFactory::buildChat(const std::string& sender, const std::string& partyId, const std::string& text)
{
    return make(MessageTypes::CHAT, sender, partyId, text);
}

Message MessageFactory::buildMemberList(
    const std::string& partyId,
    const std::vector<std::string>& members)
{
    nlohmann::json arr = members;
    return make(MessageTypes::MEMBER_LIST, "", partyId, arr.dump());
}

Message MessageFactory::buildCodeSync(
    const std::string& sender,
    const std::string& partyId,
    const std::string& code)
{
    return make(MessageTypes::CODE_SYNC, sender, partyId, code);
}

Message MessageFactory::buildTaskSubmit(
    const std::string& sender,
    const std::string& partyId,
    int taskId,
    const std::string& answer)
{
    nlohmann::json p;
    p["taskId"] = taskId;
    p["answer"] = answer;
    return make(MessageTypes::TASK_SUBMIT, sender, partyId, p.dump());
}

Message MessageFactory::buildTaskResult(const std::string& partyId, int taskId, bool passed, const std::string& feedback)
{
    nlohmann::json p;
    p["taskId"]   = taskId;
    p["passed"]   = passed;
    p["feedback"] = feedback;
    return make(MessageTypes::TASK_RESULT, "", partyId, p.dump());
}

Message MessageFactory::buildPing(const std::string& partyId)
{
    return make(MessageTypes::PING, "", partyId, "");
}

Message MessageFactory::buildPong(const std::string& partyId)
{
    return make(MessageTypes::PONG, "", partyId, "");
}

Message MessageFactory::buildAuthOk(const std::string& username)
{
    return make(MessageTypes::AUTH_OK, username, "", "");
}

Message MessageFactory::buildAuthFail(const std::string& reason)
{
    return make(MessageTypes::AUTH_FAIL, "", "", reason);
}

Message MessageFactory::buildError(
    const std::string& partyId,
    const std::string& reason)
{
    return make(MessageTypes::ERROR, "", partyId, reason);
}

nlohmann::json MessageFactory::toJson(const Message& msg)
{
    return {
        { "type",      msg.type      },
        { "sender",    msg.sender    },
        { "partyId",   msg.partyId   },
        { "payload",   msg.payload   },
        { "timestamp", msg.timestamp }
    };
}

std::string MessageFactory::toJsonString(const Message& msg)
{
    return toJson(msg).dump();
}

Message MessageFactory::fromJson(const nlohmann::json& j)
{
    Message msg;
    msg.type      = j.value("type",      "");
    msg.sender    = j.value("sender",    "");
    msg.partyId   = j.value("partyId",   "");
    msg.payload   = j.value("payload",   "");
    msg.timestamp = j.value("timestamp", 0L);
    return msg;
}

Message MessageFactory::fromJsonString(const std::string& raw)
{
    try
    {
        auto j = nlohmann::json::parse(raw);
        return fromJson(j);
    }
    catch (const nlohmann::json::exception&)
    {
        Message err;
        err.type    = MessageTypes::ERROR;
        err.payload = "malformed json: " + raw;
        return err;
    }
}
nlohmann::json MessageFactory::makeLogin(const std::string& username, const std::string& password) {
    return { {"type", "login"}, {"username", username}, {"password", password} };
}

nlohmann::json MessageFactory::makeRegister(const std::string& username, const std::string& password) {
    return { {"type", "register"}, {"username", username}, {"password", password} };
}

nlohmann::json MessageFactory::makeGetRooms() {
    return { {"type", "get_rooms"} };
}

nlohmann::json MessageFactory::makeCreateRoom(const std::string& roomName) {
    return { {"type", "create_room"}, {"name", roomName} };
}

nlohmann::json MessageFactory::makeJoinRoom(const std::string& roomId) {
    return { {"type", "join_room"}, {"room_id", roomId} };
}

nlohmann::json MessageFactory::makeLeaveRoom() {
    return { {"type", "leave_room"} };
}

nlohmann::json MessageFactory::makeCodeUpdate(const std::string& roomId, const std::string& code) {
    return { {"type", "code_update"}, {"room_id", roomId}, {"code", code} };
}

nlohmann::json MessageFactory::makeChatMsg(const std::string& roomId, const std::string& sender, const std::string& text) {
    return { {"type", "chat_message"}, {"room_id", roomId}, {"sender", sender}, {"text", text} };
}
nlohmann::json MessageFactory::makeSessionRestore(const std::string& username) {
    return { {"type", "session_restore"}, {"username", username} };
}

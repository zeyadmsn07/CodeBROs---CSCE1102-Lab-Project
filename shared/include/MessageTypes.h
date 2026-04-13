#ifndef MESSAGE_TYPES_H
#define MESSAGE_TYPES_H

#include <string>

namespace MessageTypes {
const std::string JOIN = "JOIN";
const std::string LEAVE = "LEAVE";
const std::string CHAT = "CHAT";
const std::string TYPING = "TYPING";
const std::string MEMBER_LIST = "MEMBER_LIST";
const std::string CODE_SYNC = "CODE_SYNC";
const std::string TASK_SUBMIT = "TASK_SUBMIT";
const std::string TASK_RESULT = "TASK_RESULT";
const std::string PING = "PING";
const std::string PONG = "PONG";
const std::string AUTH_OK = "AUTH_OK";
const std::string AUTH_FAIL = "AUTH_FAIL";
const std::string ERROR = "ERROR";
}  // namespace MessageTypes

#endif
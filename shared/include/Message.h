#ifndef MESSAGE_H
#define MESSAGE_H
#include <string>
#include <ctime>

struct Message
{
    std::string type;
    std::string sender;
    std::string partyId;
    std::string payload;
    long timestamp;

    Message() : timestamp(0) {}

    Message(std::string type, std::string sender, std::string partyId, std::string payload, long timestamp)
        : type(std::move(type)), sender(std::move(sender)), partyId(std::move(partyId)), payload(std::move(payload))
        , timestamp(timestamp) {}
};

#endif
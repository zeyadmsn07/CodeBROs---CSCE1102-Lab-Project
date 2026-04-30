#include <gtest/gtest.h>
#include "MessageFactory.h"
#include "MessageTypes.h"

TEST(MessageFactory, BuildChatFields)
{
    auto msg = MessageFactory::buildChat("Alice", "room1", "hi");
    EXPECT_EQ(msg.type,    MessageTypes::CHAT);
    EXPECT_EQ(msg.sender,  "Alice");
    EXPECT_EQ(msg.payload, "hi");
    EXPECT_EQ(msg.partyId, "room1");
}

TEST(MessageFactory, JsonRoundTrip)
{
    auto msg  = MessageFactory::buildChat("Omar", "ROOM1", "hey");
    auto str  = MessageFactory::toJsonString(msg);
    auto msg2 = MessageFactory::fromJsonString(str);

    EXPECT_EQ(msg2.type,    msg.type);
    EXPECT_EQ(msg2.sender,  msg.sender);
    EXPECT_EQ(msg2.payload, msg.payload);
    EXPECT_EQ(msg2.partyId, msg.partyId);
}

TEST(MessageFactory, MalformedJsonReturnsError)
{
    auto msg = MessageFactory::fromJsonString("{{{not valid");
    EXPECT_EQ(msg.type, MessageTypes::ERROR);
}
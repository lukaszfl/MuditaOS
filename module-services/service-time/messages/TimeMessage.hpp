/*
 *  @file TimeMessage.hpp
 *  @author
 *  @date 08.05.20
 *  @brief
 *  @copyright Copyright (C) 2019 mudita.com
 *  @details
 */

#ifndef PUREPHONE_TIMEMESSAGE_HPP
#define PUREPHONE_TIMEMESSAGE_HPP

#include <memory>
#include <variant>
#include "Service/Message.hpp"
#include "MessageType.hpp"

class TimeMessage : public sys::DataMessage
{
  public:
    TimeMessage(MessageType messageType) : sys::DataMessage(messageType), type(messageType){};

    virtual ~TimeMessage() = default;

    MessageType type = MessageType::MessageTypeUninitialized;
};

class CalendarReminderFiredNotificationMessage : public TimeMessage
{
  public:
    CalendarReminderFiredNotificationMessage() : TimeMessage(MessageType::CalendarReminderFired)
    {}
};

class TimeNotificationMessage : public TimeMessage
{
  public:
    // Time service will send async notifications specified below
    enum class Type
    {
        EventReminder,
        Event
    };

    TimeNotificationMessage(Type type) : TimeMessage(MessageType::TimeNotification), type(type)
    {}

    ~TimeNotificationMessage()
    {}

    Type type;
};

class TimeResponseMessage : public sys::ResponseMessage
{
  public:
    TimeResponseMessage(bool retCode,
                        std::string retdata    = std::string(),
                        MessageType responseTo = MessageType::MessageTypeUninitialized)
        : sys::ResponseMessage(sys::ReturnCodes::Success, responseTo), retCode(retCode), data(retdata){};
    TimeResponseMessage(bool retCode, MessageType responseTo)
        : sys::ResponseMessage(sys::ReturnCodes::Success, responseTo), retCode(retCode){};
    virtual ~TimeResponseMessage(){};

    bool retCode;
    std::string data;
};

class TimeStart : public TimeMessage
{
  public:
    TimeStart() : TimeMessage(MessageType::TimeStart)
    {}
};

class TimeProgress : public TimeMessage
{
  public:
    TimeProgress() : TimeMessage(MessageType::TimeProgress)
    {}
};

class TimeFinished : public TimeMessage
{
  public:
    TimeFinished() : TimeMessage(MessageType::TimeFinished)
    {}
};

#endif // PUREPHONE_SERVICETIME_HPP

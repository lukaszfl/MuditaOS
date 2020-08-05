/*
 *  @file CalendarMessage.hpp
 *  @author
 *  @date 08.05.20
 *  @brief
 *  @copyright Copyright (C) 2019 mudita.com
 *  @details
 */

#ifndef PUREPHONE_SERVICECALENDAR_HPP
#define PUREPHONE_SERVICECALENDAR_HPP

#include <memory>
#include <variant>
#include "Service/Message.hpp"
#include "MessageType.hpp"

class CalendarMessage : public sys::DataMessage
{
  public:
    CalendarMessage(MessageType messageType) : sys::DataMessage(messageType), type(messageType){};

    virtual ~CalendarMessage(){};

    MessageType type;
};

class CalendarNotificationMessage : public CalendarMessage
{
  public:
    // Calendar service will send async notifications specified below
    enum class Type
    {
        EventReminder,
        Event
    };

    CalendarNotificationMessage(Type type) : CalendarMessage(MessageType::CalendarNotification), type(type)
    {}

    ~CalendarNotificationMessage()
    {}

    Type type;
};

class CalendarStart : public CalendarMessage
{
  public:
    CalendarStart() : CalendarMessage(MessageType::CalendarStart)
    {}
};

class CalendarProgress : public CalendarMessage
{
  public:
    CalendarProgress() : CalendarMessage(MessageType::CalendarProgress)
    {}
};

class CalendarFinished : public CalendarMessage
{
  public:
    CalendarFinished() : CalendarMessage(MessageType::CalendarFinished)
    {}
};

#endif // PUREPHONE_SERVICECALENDAR_HPP

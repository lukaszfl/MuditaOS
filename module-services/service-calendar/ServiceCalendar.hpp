/*
 *  @file ServiceCalendar.hpp
 *  @author
 *  @date 08.05.20
 *  @brief
 *  @copyright Copyright (C) 2019 mudita.com
 *  @details
 */

#ifndef PUREPHONE_SERVICECALENDAR_HPP
#define PUREPHONE_SERVICECALENDAR_HPP

#include "Service/Service.hpp"
#include <functional>
#include "MessageType.hpp"

class ServiceCalendar : public sys::Service
{

  public:
    ServiceCalendar();

    ~ServiceCalendar();

    sys::Message_t DataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp = nullptr) override;

    // Invoked when timer ticked
    void TickHandler(uint32_t id) override;

    // Invoked during initialization
    sys::ReturnCodes InitHandler() override;

    sys::ReturnCodes DeinitHandler() override;

    sys::ReturnCodes SwitchPowerModeHandler(const sys::ServicePowerMode mode) override final;

    static const char *serviceName;
};

#endif // PUREPHONE_SERVICECALENDAR_HPP

/*
 * ServicePresage.hpp
 *
 *  Created on: 31 paź 2019
 *      Author: kuba
 */

#ifndef MODULE_SERVICES_SERVICE_PRESAGE_SERVICEPRESAGE_HPP_
#define MODULE_SERVICES_SERVICE_PRESAGE_SERVICEPRESAGE_HPP_

#include "bsp/keyboard/key_codes.hpp"
#include "Service/Service.hpp"
#include "Service/Message.hpp"
#include "Service/Worker.hpp"
#include "MessageType.hpp"

#include "bsp/common.hpp"
#include "../presage_cmake/lib/presage.h"


class ServicePresage: public sys::Service {
private:
	void HandleAlarmTrigger(sys::DataMessage* msgl);
	void GetNextAlarmTimestamp(time_t timestamp);
	static const char* serviceName;
protected:
	//flag informs about suspend/resume status
	bool suspended = false;
public:
	ServicePresage();
    ~ServicePresage();

    LegacyPresageCallback* callback;
    Presage* presage;

    sys::Message_t DataReceivedHandler(sys::DataMessage* msgl,sys::ResponseMessage* resp) override;

    // Invoked during initialization
    sys::ReturnCodes InitHandler() override;
    // Invoked during deinitialization
    sys::ReturnCodes DeinitHandler() override;
    // Invoked when timer ticked
	void TickHandler(uint32_t id) override;

    sys::ReturnCodes SwitchPowerModeHandler(const sys::ServicePowerMode mode) override final;

    uint32_t timer_id = 0;
};



#endif /* MODULE_SERVICES_SERVICE_PRESAGE_SERVICEPRESAGE_HPP_ */

/*
 * ServicePresage.cpp
 *
 *  Created on: 31 paź 2019
 *      Author: kuba
 */


#include "ServicePresage.hpp"

#include "log/log.hpp"



#include "vfs.hpp"

#include "service-db/api/DBServiceAPI.hpp"

const char *ServicePresage::serviceName = "ServicePresage";

ServicePresage::ServicePresage()
		: sys::Service(serviceName)
{
	LOG_INFO("[ServicePresage] Initializing");

}

ServicePresage::~ServicePresage(){

	LOG_INFO("[ServicePresage] Cleaning resources");

}

// Invoked upon receiving data message
sys::Message_t ServicePresage::DataReceivedHandler(sys::DataMessage* msgl,sys::ResponseMessage* resp) {

	bool handled = false;



	if( handled )
		return std::make_shared<sys::ResponseMessage>();
	else
		return std::make_shared<sys::ResponseMessage>(sys::ReturnCodes::Unresolved);
}

// Invoked during initialization
sys::ReturnCodes ServicePresage::InitHandler() {

	timer_id = CreateTimer(2000, true);
	ReloadTimer(timer_id);
	return sys::ReturnCodes::Success;
}

sys::ReturnCodes ServicePresage::DeinitHandler() {


	return sys::ReturnCodes::Success;
}

void ServicePresage::TickHandler(uint32_t id)
{
	LOG_INFO("Presage tick");
}

sys::ReturnCodes ServicePresage::SwitchPowerModeHandler(const sys::ServicePowerMode mode) {
    LOG_FATAL("[ServicePresage] PowerModeHandler: %d", static_cast<uint32_t>(mode));

    suspended = true;

    switch (mode){
        case sys::ServicePowerMode ::Active:
        	suspended = false;
            break;
        case sys::ServicePowerMode ::SuspendToRAM:
        case sys::ServicePowerMode ::SuspendToNVM:
            break;
    }

    return sys::ReturnCodes::Success;
}



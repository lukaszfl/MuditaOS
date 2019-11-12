/*
 * ServicePresage.cpp
 *
 *  Created on: 31 paź 2019
 *      Author: kuba
 */


#include "ServicePresage.hpp"
#include "messages/PresageMessage.hpp"
#include "MessageType.hpp"

#include "log/log.hpp"

#include "service-db/api/DBServiceAPI.hpp"

#include "../presage_cmake/lib/presage.h"

#include "ticks.hpp"

const char *ServicePresage::serviceName = "ServicePresage";



ServicePresage::ServicePresage()
		: sys::Service(serviceName)
{
	std::string config;

	LOG_INFO("[ServicePresage] Initializing");

	callback = new LegacyPresageCallback;
	presage = new Presage(callback, config);
}

ServicePresage::~ServicePresage(){

	LOG_INFO("[ServicePresage] Cleaning resources");

}

// Invoked upon receiving data message
sys::Message_t ServicePresage::DataReceivedHandler(sys::DataMessage* msgl,sys::ResponseMessage* resp) {

	bool handled = false;

	if(msgl->messageType == static_cast<uint32_t>(MessageType::PresageTick))
	{

		LOG_INFO("Presage predict");
	    const int BUFFER_SIZE = 80;
	    char buffer[ BUFFER_SIZE ];

	    memset(buffer, 0, BUFFER_SIZE);

	    buffer[0] = 'a';
	    callback->update(buffer);

	    auto timestamp = cpp_freertos::Ticks::GetTicks();
	    auto predited = presage->predict();

	    LOG_DEBUG("Presage prediction time: %lu",cpp_freertos::Ticks::GetTicks()-timestamp);
	}

	if( handled )
		return std::make_shared<sys::ResponseMessage>();
	else
		return std::make_shared<sys::ResponseMessage>(sys::ReturnCodes::Unresolved);
}

// Invoked during initialization
sys::ReturnCodes ServicePresage::InitHandler() {

	timer_id = CreateTimer(10000, true);
	ReloadTimer(timer_id);
	return sys::ReturnCodes::Success;
}

sys::ReturnCodes ServicePresage::DeinitHandler() {


	return sys::ReturnCodes::Success;
}

void ServicePresage::TickHandler(uint32_t id)
{
	LOG_INFO("Presage tick");
	auto message = std::make_shared<spresage::PresageTickMessage>(MessageType::PresageTick);
	sys::Bus::SendUnicast(message, "ServicePresage", this);
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



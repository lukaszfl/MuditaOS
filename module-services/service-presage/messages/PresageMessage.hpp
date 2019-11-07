/*
 * PresageMessage.hpp
 *
 *  Created on: 4 lis 2019
 *      Author: kuba
 */

#ifndef MODULE_SERVICES_SERVICE_PRESAGE_MESSAGES_PRESAGEMESSAGE_HPP_
#define MODULE_SERVICES_SERVICE_PRESAGE_MESSAGES_PRESAGEMESSAGE_HPP_


#include <string>
#include "Service/Message.hpp"
#include "MessageType.hpp"
#include "SwitchData.hpp"
#include "Service/Service.hpp"
#include "MessageType.hpp"


namespace spresage {




class PresageTickMessage : public sys::DataMessage
{
public:
	PresageTickMessage(MessageType messageType) : DataMessage(static_cast<uint32_t>(messageType))
	{
		type = Type::Data;

	}

	sys::Message_t Execute(sys::Service* service)
	{
		// Ignore incoming data message if this service is not yet initialized
		if(service->isReady){
			return service->DataReceivedHandler(this,nullptr);
		}
		else{
			return std::make_shared<sys::ResponseMessage>();
		}

	}

};

}
#endif /* MODULE_SERVICES_SERVICE_PRESAGE_MESSAGES_PRESAGEMESSAGE_HPP_ */

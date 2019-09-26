/*
 * @file MessagesSwitchData.hpp
 * @author Robert Borzecki (robert.borzecki@mudita.com)
 * @date 26 wrz 2019
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */
#ifndef MODULE_APPS_APPLICATION_MESSAGES_DATA_MESSAGESSWITCHDATA_HPP_
#define MODULE_APPS_APPLICATION_MESSAGES_DATA_MESSAGESSWITCHDATA_HPP_

#include <string>
#include "SwitchData.hpp"
#include "Interface/ThreadRecord.hpp"

namespace app {

class MessagesSwitchData: public gui::SwitchData {
public:
	enum class Type {
		UNDEFINED,
		PROVIDE_THREAD,
	};
protected:
	Type type = Type::UNDEFINED;
public:
	MessagesSwitchData(app::MessagesSwitchData::Type type ) : type{ type } {};
	virtual ~MessagesSwitchData(){};

	const Type& getType() const { return type; };
};

class ThreadSwitchData: public MessagesSwitchData {
protected:
	std::shared_ptr<ThreadRecord> threadRecord = nullptr;
public:
	ThreadSwitchData( std::shared_ptr<ThreadRecord> threadRecord ) :
		MessagesSwitchData( app::MessagesSwitchData::Type::PROVIDE_THREAD ), threadRecord{ threadRecord }{};
	virtual ~ThreadSwitchData(){};

	std::shared_ptr<ThreadRecord> getThread() const { return threadRecord; };
};

} /* namespace app */

#endif /* MODULE_APPS_APPLICATION_MESSAGES_DATA_MESSAGESSWITCHDATA_HPP_ */

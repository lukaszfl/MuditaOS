// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <ApplicationCommon.hpp>
#include <apps-common/messages/DialogMetadataMessage.hpp>
#include <common/TimeUtils.hpp>
#include <common/popups/AlarmActivatedWindow.hpp>
#include <popups/Popups.hpp>
#include <service-time/AlarmMessage.hpp>
#include <service-time/Constants.hpp>

namespace gui
{
    AlarmActivatedWindow::AlarmActivatedWindow(app::ApplicationCommon *app)
        : Dialog(app, popup::window::alarm_activated_window), app::AsyncCallbackReceiver{app}
    {}

    void AlarmActivatedWindow::onBeforeShow(ShowMode mode, [[maybe_unused]] SwitchData *data)
    {
        auto task = app::AsyncRequest::createFromMessage(
            std::make_unique<alarms::AlarmGetNextSingleEventsRequestMessage>(), service::name::service_time);
        auto onResponseCallback = [this, mode](auto response) {
            auto result = dynamic_cast<alarms::AlarmGetNextSingleEventsResponseMessage *>(response);
            if (result == nullptr || result->retCode != sys::ReturnCodes::Success) {
                LOG_WARN("Get next single event request failed!");
                return false;
            }
            auto singleEvents = result->singleEvents;
            if (singleEvents.empty()) {
                LOG_WARN("There is no event!");
                return false;
            }
            const auto &startDate = singleEvents.front().startDate;
            LOG_DEBUG("Alarm time: %s", TimePointToString(startDate).c_str());
            const auto alarmTime = std::chrono::system_clock::to_time_t(startDate);
            auto msg             = std::make_unique<DialogMetadataMessage>(
                gui::DialogMetadata{.title = "",
                                    .icon  = "bell_alarm_activated",
                                    .text  = utils::time::getBottomDescription(utils::time::calculateTimeDifference(
                                                     alarmTime, utils::time::getCurrentTime()))});
            Dialog::onBeforeShow(mode, msg.get());
            statusBar->setVisible(false);
            header->setTitleVisibility(false);
            bottomBar->setActive(BottomBar::Side::RIGHT, false);
            return true;
        };
        task->execute(this->application, this, onResponseCallback);

        auto msg =
            std::make_unique<DialogMetadataMessage>(gui::DialogMetadata{.title = "", .icon = "bell_alarm_activated"});
        Dialog::onBeforeShow(mode, msg.get());
    }
} /* namespace gui */

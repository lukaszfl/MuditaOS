
#include <memory>
#include <list>

#include "../module-gui/gui/core/ImageManager.hpp"
#include "log/log.hpp"
#include "memory/usermem.h"
#include "ticks.hpp"
#include <source/version.hpp>

// module-applications
#include "application-antenna/ApplicationAntenna.hpp"
#include "application-call/ApplicationCall.hpp"
#include "application-calllog/ApplicationCallLog.hpp"
#include "application-clock/ApplicationClock.hpp"
#include "application-desktop/ApplicationDesktop.hpp"
#include "application-messages/ApplicationMessages.hpp"
#include "application-notes/ApplicationNotes.hpp"
#include "application-phonebook/ApplicationPhonebook.hpp"
#include "application-settings/ApplicationSettings.hpp"
#include "application-settings-new/ApplicationSettings.hpp"
#include "application-special-input/AppSpecialInput.hpp"
#include "application-calendar/ApplicationCalendar.hpp"
#include "application-music-player/ApplicationMusicPlayer.hpp"

// module-services
#include "service-appmgr/ApplicationManager.hpp"
#include "service-audio/ServiceAudio.hpp"
#include "service-audio/api/AudioServiceAPI.hpp"
#include "service-bluetooth/ServiceBluetooth.hpp"
#include "service-cellular/ServiceCellular.hpp"
#include "service-cellular/api/CellularServiceAPI.hpp"
#include "service-db/ServiceDB.hpp"
#include "service-db/api/DBServiceAPI.hpp"
#include "service-desktop/ServiceDesktop.hpp"
#include "service-evtmgr/Constants.hpp"
#include "service-evtmgr/EventManager.hpp"
#include "service-lwip/ServiceLwIP.hpp"
#include "service-fota/ServiceFota.hpp"
#include "service-antenna/ServiceAntenna.hpp"

// module-bsp
#include "bsp/bsp.hpp"
#include "bsp/rtc/rtc.hpp"
#include "bsp/keyboard/keyboard.hpp"

// module-vfs
#include "vfs.hpp"

// module-sys
#include "SystemManager/SystemManager.hpp"

// libphonenumber
#include <phonenumbers/phonenumberutil.h>

class vfs vfs;

int main()
{

#if SYSTEM_VIEW_ENABLED
    SEGGER_SYSVIEW_Conf();
    SEGGER_SYSVIEW_DisableEvents(SYSVIEW_EVTMASK_ISR_ENTER);
    SEGGER_SYSVIEW_DisableEvents(SYSVIEW_EVTMASK_ISR_EXIT);
    SEGGER_SYSVIEW_DisableEvents(SYSVIEW_EVTMASK_ISR_TO_SCHEDULER);
    SEGGER_SYSVIEW_WaitForConnection();
    SEGGER_SYSVIEW_Start();
#endif

    bsp::BoardInit();

    LOG_PRINTF("Launching PurePhone \n");
    LOG_PRINTF("commit: %s tag: %s branch: %s\n", GIT_REV, GIT_TAG, GIT_BRANCH);

#if 1
    auto sysmgr = std::make_shared<sys::SystemManager>(5000);

    sysmgr->StartSystem([sysmgr]() -> int {
        /// force initialization of PhonenumberUtil because of its stack usage
        /// otherwise we would end up with an init race and PhonenumberUtil could
        /// be initiated in a task with stack not big enough to handle it
        i18n::phonenumbers::PhoneNumberUtil::GetInstance();

        vfs.Init();

        bool ret = false;

        ret |=
            sys::SystemManager::CreateService(std::make_shared<EventManager>(service::name::evt_manager), sysmgr.get());
        ret |= sys::SystemManager::CreateService(std::make_shared<ServiceDB>(), sysmgr.get());
//        ret |= sys::SystemManager::CreateService(std::make_shared<ServiceAntenna>(), sysmgr.get());
#if defined(TARGET_Linux) && not defined(SERIAL_PORT)
        // For now disable pernamenlty Service cellular when there is no GSM configured
        LOG_INFO("ServiceCellular (GSM) - Disabled");
#else
        LOG_INFO("ServiceCellular (GSM) - Enabling");
        ret |= sys::SystemManager::CreateService(std::make_shared<ServiceCellular>(), sysmgr.get());
        ret |= sys::SystemManager::CreateService(std::make_shared<FotaService::Service>(), sysmgr.get());
#endif
        ret |= sys::SystemManager::CreateService(std::make_shared<ServiceAudio>(), sysmgr.get());
        ret |= sys::SystemManager::CreateService(std::make_shared<ServiceBluetooth>(), sysmgr.get());
        ret |= sys::SystemManager::CreateService(std::make_shared<ServiceLwIP>(), sysmgr.get());

        // Service Desktop disabled on master - pulling read on usb driver
        // ret |= sys::SystemManager::CreateService(std::make_shared<ServiceDesktop>(), sysmgr.get());

        // vector with launchers to applications
        std::vector<std::unique_ptr<app::ApplicationLauncher>> applications;
#ifdef ENABLE_APP_DESKTOP
        applications.push_back(app::CreateLauncher<app::ApplicationDesktop>(app::name_desktop, false));
#endif
#ifdef ENABLE_APP_CALL
        applications.push_back(app::CreateLauncher<app::ApplicationCall>(app::name_call, false));
#endif
#ifdef ENABLE_APP_SETTINGS
        applications.push_back(app::CreateLauncher<app::ApplicationSettings>(app::name_settings));
        applications.push_back(app::CreateLauncher<app::ApplicationSettingsNew>(app::name_settings_new));
#endif
#ifdef ENABLE_APP_NOTES
        applications.push_back(app::CreateLauncher<app::ApplicationNotes>(app::name_notes));
#endif
#ifdef ENABLE_APP_CALLLOG
        applications.push_back(app::CreateLauncher<app::ApplicationCallLog>(app::CallLogAppStr));
#endif
#ifdef ENABLE_APP_PHONEBOOK
        applications.push_back(app::CreateLauncher<app::ApplicationPhonebook>(app::name_phonebook));
#endif
#ifdef ENABLE_APP_MESSAGES
        applications.push_back(app::CreateLauncher<app::ApplicationMessages>(app::name_messages));
#endif
#ifdef ENABLE_APP_SPECIAL_INPUT
        applications.push_back(app::CreateLauncher<app::AppSpecialInput>(app::special_input, false));
#endif
#ifdef ENABLE_APP_ANTENNA
        applications.push_back(app::CreateLauncher<app::ApplicationAntenna>(app::name_antenna));
#endif
#ifdef ENABLE_APP_CALENDAR
        applications.push_back(app::CreateLauncher<app::ApplicationCalendar>(app::name_calendar));
#endif
#ifdef ENABLE_APP_MUSIC_PLAYER
        applications.push_back(app::CreateLauncher<app::ApplicationMusicPlayer>(app::name_music_player));
#endif

        // start application manager
        ret |= sysmgr->CreateService(
            std::make_shared<sapm::ApplicationManager>("ApplicationManager", sysmgr.get(), applications), sysmgr.get());

        if (ret) {
            return 0;
        }

        return 0;
    });

    cpp_freertos::Thread::StartScheduler();
#endif
    return 1;
}

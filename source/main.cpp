// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "config.h"

// applications
#include <application-antenna/ApplicationAntenna.hpp>
#include <application-call/ApplicationCall.hpp>
#include <application-calllog/ApplicationCallLog.hpp>
#include <application-desktop/ApplicationDesktop.hpp>
#include <application-messages/ApplicationMessages.hpp>
#include <application-notes/ApplicationNotes.hpp>
#include <application-phonebook/ApplicationPhonebook.hpp>
#include <application-settings/ApplicationSettings.hpp>
#include <application-settings-new/ApplicationSettings.hpp>
#include <application-special-input/ApplicationSpecialInput.hpp>
#include <application-calendar/ApplicationCalendar.hpp>
#include <application-music-player/ApplicationMusicPlayer.hpp>
#include <application-meditation/ApplicationMeditation.hpp>
#include <application-calculator/ApplicationCalculator.hpp>
#include <application-alarm-clock/ApplicationAlarmClock.hpp>

// services
#include <service-appmgr/model/ApplicationManager.hpp>
#include <service-audio/ServiceAudio.hpp>
#include <service-bluetooth/ServiceBluetooth.hpp>
#include <service-db/ServiceDB.hpp>
#include <service-evtmgr/Constants.hpp>
#include <service-evtmgr/EventManager.hpp>
#include <service-lwip/ServiceLwIP.hpp>
#include <service-time/ServiceTime.hpp>
#include <service-fileindexer/Constants.hpp>
#include <service-fileindexer/ServiceFileIndexer.hpp>

#if ENABLE_GSM == 1
#include <service-fota/ServiceFota.hpp>
#include <service-cellular/ServiceCellular.hpp>
#include <service-antenna/ServiceAntenna.hpp>
#endif

#include <bsp/bsp.hpp>
#include <Application.hpp>
#include <ApplicationLauncher.hpp>
#include <log/log.hpp>
#include <phonenumbers/phonenumberutil.h>
#include <source/version.hpp>
#include <SystemManager/SystemManager.hpp>
#include <thread.hpp>
#include <purefs/vfs_subsystem.hpp>

#include <memory>
#include <vector>

#define TICKS_TO_MSEC(tick) ((uint32_t)((uint64_t)(tick)*1000uL / (uint64_t)configTICK_RATE_HZ))
#include <fcntl.h>

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

    auto sysmgr = std::make_shared<sys::SystemManager>(5000);
    purefs::subsystem::vfs_handle_t vfs;

    ///////////////////////////////////////////////

    sysmgr->StartSystem([sysmgr, &vfs]() {
        /// force initialization of PhonenumberUtil because of its stack usage
        /// otherwise we would end up with an init race and PhonenumberUtil could
        /// be initiated in a task with stack not big enough to handle it
        i18n::phonenumbers::PhoneNumberUtil::GetInstance();
        vfs     = purefs::subsystem::initialize();
        int err = purefs::subsystem::mount_defaults();

        if (err) {
            LOG_FATAL("VFS subystem fatal error %i", err);
            std::abort();
        }

        //////////// FILESYSTEM ONE-SHOT TEST //////////////
        {
            // SETTINGS
            auto testSizeInSectors = 10000;     // size of the test buffer in MMC sectors
            auto firstTestSector   = 2048 * 10; // first sector to write to
            auto deviceName        = "emmc0";
            auto testFileName      = "testfile.crap";
            auto lfsTestFileName   = std::string(purefs::dir::getUserDiskPath() / testFileName);
            auto fatTestFileName   = std::string(purefs::dir::getRootDiskPath() / testFileName);

            auto diskManager = std::get<0>(vfs);
            auto fs          = std::get<1>(vfs);

            const auto handle     = diskManager->device_handle(deviceName);
            const auto sectorSize = diskManager->get_info(handle, purefs::blkdev::info_type::sector_size);

            auto writeSize = sectorSize * testSizeInSectors;
            LOG_DEBUG("Test size %ld MB", (long)writeSize / 1024 / 1024);

            // std::unique_ptr<char[]> testPatternBuf(new char[writeSize]);
            std::unique_ptr<char[]> workBuf(new char[writeSize]);

            enum class BufOp
            {
                Fill,
                Compare,
                Clear
            };
            auto bufferOperation = [](BufOp op, char *buf, size_t writeSize) {
                if (op == BufOp::Clear) {
                    std::memset(buf, 0, writeSize);
                    return true;
                }

                // attempts to use non-trivial pattern to fill
                char writePiece  = 0;
                auto noiseOffset = 1;
                for (size_t idx = 0; idx < writeSize; idx++) {
                    if (writePiece == 0xFF) {
                        for (int nidx = 0; (nidx < noiseOffset) && (idx < writeSize); nidx++) {
                            if (op == BufOp::Fill) {
                                buf[idx] = noiseOffset & 0xFF;
                            }
                            else if (buf[idx] != (noiseOffset & 0xFF)) {
                                return false;
                            }
                            idx++;
                        }
                        noiseOffset++;
                    }
                    if (op == BufOp::Fill) {
                        buf[idx] = writePiece;
                    }
                    else if (buf[idx] != writePiece) {
                        return false;
                    }
                    writePiece++;
                };
                return true;
            };

            auto vfsTest = [&fs, &workBuf, &writeSize, &bufferOperation](std::string system, std::string path) {
                // VFS TEST littlefs
                auto file = fs->open(path, O_CREAT | O_RDWR, S_IRWXU);
                if (file <= 0) {
                    LOG_ERROR("Failed creating test file %s.", path.c_str());
                    return false;
                }

                bufferOperation(BufOp::Fill, workBuf.get(), writeSize);
                auto start = xTaskGetTickCount();
                fs->write(file, workBuf.get(), writeSize);
                auto tickCount           = xTaskGetTickCount() - start;
                auto speedInBytesPerMsec = writeSize / TICKS_TO_MSEC(tickCount);
                LOG_DEBUG("VFAT %s write speed %ld B/ms", system.c_str(), (long)(speedInBytesPerMsec));
                fs->close(file);

                file = fs->open(path, O_RDWR, S_IRWXU);
                if (file <= 0) {
                    LOG_ERROR("Failed creating test file %s.", path.c_str());
                    return false;
                }

                bufferOperation(BufOp::Clear, workBuf.get(), writeSize);
                start         = xTaskGetTickCount();
                auto sizeRead = fs->read(file, workBuf.get(), writeSize);
                tickCount     = xTaskGetTickCount() - start;
                if (sizeRead != writeSize) {
                    LOG_ERROR("Error reading %s.", system.c_str());
                }
                speedInBytesPerMsec = writeSize / TICKS_TO_MSEC(tickCount);
                LOG_DEBUG("VFAT %s read speed %ld B/ms", system.c_str(), (long)(speedInBytesPerMsec));

                if (bufferOperation(BufOp::Compare, workBuf.get(), writeSize) == false) {
                    LOG_ERROR("Bad write on %s.", system.c_str());
                    return true;
                }

                fs->close(file);
                return true;
            };

            LOG_DEBUG("Testing LFS file %s", std::string(lfsTestFileName).c_str());
            vfsTest("LFS", std::string(lfsTestFileName));

            LOG_DEBUG("Testing FAT file %s", std::string(fatTestFileName).c_str());
            vfsTest("FAT", std::string(fatTestFileName));

            bufferOperation(BufOp::Fill, workBuf.get(), writeSize);

            auto start = xTaskGetTickCount();
            diskManager->write(handle, workBuf.get(), firstTestSector, testSizeInSectors);
            auto tickCount                = xTaskGetTickCount() - start;
            auto writeSpeedInBytesPerMsec = writeSize / TICKS_TO_MSEC(tickCount);
            LOG_DEBUG("MMC write speed %ld B/ms", (long)(writeSpeedInBytesPerMsec));

            bufferOperation(BufOp::Clear, workBuf.get(), writeSize);
            start = xTaskGetTickCount();
            diskManager->read(handle, workBuf.get(), firstTestSector, testSizeInSectors);
            tickCount                    = xTaskGetTickCount() - start;
            auto readSpeedInBytesPerMsec = writeSize / TICKS_TO_MSEC(tickCount);
            LOG_DEBUG("MMC read speed %ld B/ms", (long)(readSpeedInBytesPerMsec));

            bufferOperation(BufOp::Compare, workBuf.get(), writeSize);
            if (bufferOperation(BufOp::Compare, workBuf.get(), writeSize) == false) {
                LOG_ERROR("Bad write on MMC");
            }

            return true;
        }

        auto ret = true;
        ret &=
            sys::SystemManager::CreateService(std::make_shared<EventManager>(service::name::evt_manager), sysmgr.get());
#if ENABLE_FILEINDEXER_SERVICE
        ret &= sys::SystemManager::CreateService(
            std::make_shared<service::ServiceFileIndexer>(service::name::file_indexer), sysmgr.get());
#endif
        ret &= sys::SystemManager::CreateService(std::make_shared<ServiceDB>(), sysmgr.get());

#if ENABLE_GSM == 0
        // For now disable pernamenlty Service cellular when there is no GSM configured
        LOG_INFO("ServiceCellular (GSM) - Disabled");
#else
        LOG_INFO("ServiceCellular (GSM) - Enabling");
        ret &= sys::SystemManager::CreateService(std::make_shared<ServiceAntenna>(), sysmgr.get());
        ret &= sys::SystemManager::CreateService(std::make_shared<ServiceCellular>(), sysmgr.get());
        ret &= sys::SystemManager::CreateService(std::make_shared<FotaService::Service>(), sysmgr.get());
#endif

        ret &= sys::SystemManager::CreateService(std::make_shared<ServiceAudio>(), sysmgr.get());
        ret &= sys::SystemManager::CreateService(std::make_shared<ServiceBluetooth>(), sysmgr.get());
        ret &= sys::SystemManager::CreateService(std::make_shared<ServiceLwIP>(), sysmgr.get());
        ret &= sys::SystemManager::CreateService(std::make_shared<ServiceDesktop>(), sysmgr.get());

        ret &= sys::SystemManager::CreateService(std::make_shared<stm::ServiceTime>(), sysmgr.get());

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
#endif
#ifdef ENABLE_APP_SETTINGS_NEW
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
        applications.push_back(app::CreateLauncher<app::ApplicationSpecialInput>(app::special_input, false));
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
#ifdef ENABLE_APP_MEDITATION
        applications.push_back(app::CreateLauncher<app::ApplicationMeditation>(app::name_meditation));
#endif
#ifdef ENABLE_APP_CALCULATOR
        applications.push_back(app::CreateLauncher<app::ApplicationCalculator>(app::name_calculator));
#endif
#ifdef ENABLE_APP_ALARM_CLOCK
        applications.push_back(app::CreateLauncher<app::ApplicationAlarmClock>(app::name_alarm_clock));
#endif

        // start application manager
        ret &= sysmgr->CreateService(
            std::make_shared<app::manager::ApplicationManager>(
                app::manager::ApplicationManager::ServiceName, std::move(applications), app::name_desktop),
            sysmgr.get());

        return ret;
    });
    LOG_PRINTF("Launching PurePhone \n");
    LOG_PRINTF("commit: %s tag: %s branch: %s\n", GIT_REV, GIT_TAG, GIT_BRANCH);

    cpp_freertos::Thread::StartScheduler();

    return 1;
}

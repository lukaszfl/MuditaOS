// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

class ConnectionManagerCellularCommandsInterface
{
  public:
    /**
     * Destructor
     */
    virtual ~ConnectionManagerCellularCommandsInterface()
    {}
    /**
     * It's enabling modem radio module, so it can connect to the GSM network
     * @return true on success, false on fail
     */
    virtual auto disconnectFromNetwork() -> bool = 0;
    /**
     * It's enabling modem radio module, so it can connect to the GSM network
     * @return true on success, false on fail
     */
    virtual auto connectToNetwork() -> bool = 0;
    /**
     * * Checks current state of modem radio module
     * @return true when modem radio module is enabled, false when modem radio module is disabled
     */
    virtual auto isConnectedToNetwork() -> bool = 0;
    /**
     * * It's disabling modem radio module, so it can't connect to the GSM network
     * @return true on success, false on fail
     */
    virtual auto clearNetworkIndicator() -> bool = 0;
    /**
     * @brief Checks if there is ongoing call and terminates it
     * @return true on success, false on fail
     */
    virtual auto hangUpOngoingCall() -> bool = 0;
    /**
     * @brief Checks if connection Timer is active
     * @return true when timer is active, false when not
     */
    virtual auto isConnectionTimerActive() -> bool = 0;
    /**
     * @brief Stats connectionTimer
     */
    virtual void startConnectionTimer() = 0;
    /**
     * @brief Stops connectionTimer
     */
    virtual void stopConnectionTimer() = 0;
};

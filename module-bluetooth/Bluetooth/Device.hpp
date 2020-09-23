#pragma once
#include <string>
#include <cstring>
#include <module-bluetooth/lib/btstack/src/bluetooth.h>

struct Device
{
  public:
    Device(std::string name = "") : name(name)
    {}
    virtual ~Device(){};
    std::string name;
};

enum DEVICE_STATE
{
    REMOTE_NAME_REQUEST,
    REMOTE_NAME_INQUIRED,
    REMOTE_NAME_FETCHED
};

struct Devicei : public Device
{
  public:
    bd_addr_t address;
    uint8_t pageScanRepetitionMode;
    uint16_t clockOffset;
    enum DEVICE_STATE state;

    Devicei(std::string name = "") : name(name)
    {}
    virtual ~Devicei()
    {}
    void address_set(bd_addr_t *addr)
    {
        memcpy(&address, addr, sizeof address);
    }
    std::string name;
};

#pragma once

#include "Profile.hpp"
#include <memory>
extern "C"
{
#include <module-bluetooth/lib/btstack/src/bluetooth.h>
};
namespace Bt
{

    class SCO
    {
      public:
        SCO();
        ~SCO();

        SCO(const SCO &other);
        auto operator=(SCO rhs) -> SCO &;

        void close();
        void setCodec(uint8_t codec);
        void init();
        void send(hci_con_handle_t sco_handle);
        void receive(uint8_t *packet, uint16_t size);

      private:
        class SCOImpl;
        std::unique_ptr<SCOImpl> pimpl;
    };
} // namespace Bt
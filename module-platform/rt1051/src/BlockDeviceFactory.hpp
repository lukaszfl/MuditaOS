#include <purefs/DeviceFactory.hpp>

#include <memory>

namespace platform::rt1051
{
    class BlockDeviceFactory : public purefs::DeviceFactory
    {
      public:
        std::unique_ptr<purefs::blkdev::disk> makeDefaultBlockDevice() override final;
        std::unique_ptr<purefs::blkdev::disk> makeDefaultNvmDevice() override final;
    };
}; // namespace platform::rt1051

#include <purefs/DeviceFactory.hpp>

#include <memory>

namespace platform::linux::purefs
{
    class DeviceFactory : public purefs::DeviceFactory
    {
      public:
        std::unique_ptr<blkdev::disk> makeDefaultBlockDevice() override final;
        std::unique_ptr<blkdev::disk> makeDefaultNvmDevice() override final;
    };
}; // namespace platform::linux::purefs

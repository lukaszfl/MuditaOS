#include "BlockDeviceFactory.hpp"
#include "DiskImage.hpp"

using platform::linux::BlockDeviceFactory;

BlockDeviceFactory::BlockDeviceFactory(std::string imageName) : imageName(std::move(imageName))
{}

std::unique_ptr<purefs::blkdev::disk> BlockDeviceFactory::makeDefaultBlockDevice()
{
    return std::make_unique<purefs::blkdev::disk_image>(imageName);
}

std::unique_ptr<purefs::blkdev::disk> BlockDeviceFactory::makeDefaultNvmDevice()
{
    return std::make_unique<purefs::blkdev::disk_image>(eeprom_image_name, eeprom_sector_size, eeprom_num_sysparts);
}

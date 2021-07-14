#include "LinuxPlatform.hpp"
#include "BlockDeviceFactory.hpp"

#include <bsp/bsp.hpp>
#include <purefs/vfs_subsystem.hpp>

#include <exception>

using platform::linux::BlockDeviceFactory;
using platform::linux::LinuxPlatform;

LinuxPlatform::LinuxPlatform(std::string imageName) : imageName(std::move(imageName))
{
    bsp::BoardInit();
}

LinuxPlatform::~LinuxPlatform()
{
    if (usesFilesystem) {
        purefs::subsystem::unmount_all();
    }
}

void LinuxPlatform::initFilesystem()
{
    if (usesFilesystem) {
        throw std::runtime_error("Filesystem already initialized");
    }

    auto blockDeviceFactory = std::make_unique<BlockDeviceFactory>(imageName);
    vfs                     = purefs::subsystem::initialize(std::move(blockDeviceFactory));

    if (int err = purefs::subsystem::mount_defaults(); err != 0) {
        throw std::runtime_error("Failed to initiate filesystem: " + std::to_string(err));
    }

    usesFilesystem = true;
}

#pragma once

#include <Platform.hpp>

#include <purefs/vfs_subsystem.hpp>

#include <string>

namespace platform::linux
{
    class LinuxPlatform : public platform::Platform
    {
      public:
        explicit LinuxPlatform(std::string imageName);
        ~LinuxPlatform();

        void initFilesystem() override final;

      private:
        bool usesFilesystem = false;
        std::string imageName;
        purefs::subsystem::vfs_handle_t vfs;
    };

} // namespace platform::linux

#pragma once

#include <Platform.hpp>

#include <purefs/vfs_subsystem.hpp>

#include <string>

namespace platform::rt1051
{
    class RT1051Platform : public platform::Platform
    {
      public:
        RT1051Platform();
        ~RT1051Platform();

        void initFilesystem() override final;

      private:
        bool usesFilesystem = false;
        purefs::subsystem::vfs_handle_t vfs;
    };

} // namespace platform::rt1051

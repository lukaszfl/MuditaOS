#pragma once

namespace platform
{
    class Platform
    {
      public:
        virtual ~Platform()           = default;
        virtual void initFilesystem() = 0;
    };
} // namespace platform

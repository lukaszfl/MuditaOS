#pragma once

#include <Platform.hpp>

#include <memory>

namespace purephone
{
    class PlatformFactory
    {
      public:
        constexpr static auto imageName = "PurePhone.img";

        auto makePlatform() const -> std::unique_ptr<platform::Platform>;
    };
}; // namespace purephone

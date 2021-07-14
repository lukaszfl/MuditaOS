#include "PlatformFactory.hpp"

#if defined(PLATFORM_linux)
#include <LinuxPlatform.hpp>
#endif

#if defined(PLATFORM_rt1051)
#include <RT1051Platform.hpp>
#endif

#include <memory>
#include <utility>

using purephone::PlatformFactory;

auto PlatformFactory::makePlatform() const -> std::unique_ptr<platform::Platform>
{
#if defined(PLATFORM_linux)
    return std::make_unique<platform::linux::LinuxPlatform>(imageName);
#elif defined(PLATFORM_rt1051)
    return std::make_unique<platform::rt1051::RT1051Platform>();
#else
#error "Target platform not defined."
#endif
    return nullptr;
}

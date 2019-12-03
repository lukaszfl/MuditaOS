#pragma once

#include <Style.hpp>

namespace calculatorAppStyle
{

// numberLabel
namespace numberLabel
{
constexpr uint32_t x = 60;
constexpr uint32_t y = 157;
constexpr uint32_t w = style::window_width - 2 * x;
constexpr uint32_t h = 66;
constexpr uint32_t borderW = 1;
} // namespace numberLabel

// ICON
namespace icon
{
constexpr uint32_t w = 100;
constexpr uint32_t h = 100;
namespace label
{
constexpr uint32_t x = 9;
constexpr uint32_t y = 58;
constexpr uint32_t w = 82;
constexpr uint32_t h = 20;
} // namespace label
namespace img
{
constexpr uint32_t x = 34;
constexpr uint32_t y = 15;
constexpr uint32_t w = 32;
constexpr uint32_t h = 32;
} // namespace img
} // namespace icon

} // namespace calculatorAppStyle
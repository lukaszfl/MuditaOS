#include "RectProgressBar.hpp"
#include "BoxLayout.hpp"
#include "Rect.hpp"
#include "log/log.hpp"

namespace gui 
{
    RectProgressBar::RectProgressBar(
        unsigned int number_of_rects, Length height, Length width, Length rect_width, OrientationFlags orientation)
        : ProgressBar(nullptr, 0, 0, height, width), number_of_rects(number_of_rects), orientation(orientation),
          rect_width(rect_width)
    {
        bool is_horizontal = orientation == OrientationFlags::GUI_ORIENTATION_HORIZONTAL;

        auto makebox = [this, height, width](bool is_horizontal) -> BoxLayout * {
            if (is_horizontal) {
                return new HBox(this, 0, 0, height, width);
            }
            return new VBox(this, 0, 0, height, width);
        };

        layout = makebox(is_horizontal);

        if (rect_width == 0) {
            LOG_ERROR("rect width can't be 0! set to 1 at minimum");
            rect_width = 1;
        }

        Length space_taken  = number_of_rects * rect_width;
        Length size_in_axis = getSize(is_horizontal ? Axis::X : Axis::Y);
        auto size_left      = size_in_axis - space_taken;
        Length margin       = 0;

        if (size_left <= 0) {
            LOG_ERROR("rects will take too much space and wont fit in");
        }
        else {
            margin = size_left / (number_of_rects - 1);
        }

        auto [w, h] =
            std::pair<Length, Length>(is_horizontal ? rect_width : width, is_horizontal ? height : rect_width);

        for (unsigned int i = 0; i < number_of_rects; ++i) {
            auto rect = new Rect(layout, 0, 0, w, h);
            rect->setRadius(2);
            rect->setFillColor(ColorGrey);
        }
    }
} // namespace gui

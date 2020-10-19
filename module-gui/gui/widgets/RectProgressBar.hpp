#pragma once

#include "BoxLayout.hpp"
#include "Common.hpp"
#include "ProgressBar.hpp"

namespace gui 
{
    class RectProgressBar : public ProgressBar
    {
        private:
            unsigned int number_of_rects = number_of_rects = 0;
            OrientationFlags orientation = OrientationFlags::GUI_ORIENTATION_VERTICAL;
            BoxLayout* layout = nullptr;
            Length rect_width = 0;
        public:
        RectProgressBar(unsigned int number_of_rects, Length height, Length width, Length rect_width, OrientationFlags orientation);
    };
}

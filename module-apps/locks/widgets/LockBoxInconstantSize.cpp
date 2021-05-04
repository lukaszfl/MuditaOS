// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "Lock.hpp"
#include "LockBoxInconstantSize.hpp"

#include <locks/data/LockStyle.hpp>
#include <locks/windows/LockInputWindow.hpp>

namespace gui
{
    void LockBoxInconstantSize::buildLockBox(unsigned int pinSize)
    {
        buildPinLabels(0);
    }

    void LockBoxInconstantSize::clear()
    {
        lockWindow->pinLabelsBox->erase();
        buildPinLabels(0);
    }

    void LockBoxInconstantSize::popChar(unsigned int charNum)
    {
        buildPinLabels(charNum);
    }

    void LockBoxInconstantSize::putChar(unsigned int charNum)
    {
        buildPinLabels(++charNum);
    }

    void LockBoxInconstantSize::buildPinLabels(unsigned int pinSize)
    {
        if (lockWindow->pinLabelsBox) {
            lockWindow->pinLabelsBox->erase();
        }

        auto itemBuilder = []() {
            auto label = new gui::Image("dot_12px_hard_alpha_W_G");
            return label;
        };

        lockWindow->buildPinLabels(itemBuilder, pinSize);
        lockWindow->pinLabelsBox->setEdges(RectangleEdge::Bottom);
    }
} // namespace gui

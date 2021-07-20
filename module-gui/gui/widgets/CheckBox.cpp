// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "CheckBox.hpp"
#include "Style.hpp"
#include <InputEvent.hpp>
#include <i18n/i18n.hpp>

namespace gui
{

    CheckBox::CheckBox(Item *parent,
                       const uint32_t &x,
                       const uint32_t &y,
                       const uint32_t &w,
                       const uint32_t &h,
                       const std::function<void(const UTF8 &text)> &bottomBarTemporaryMode,
                       const std::function<void()> &bottomBarRestoreFromTemporaryMode,
                       BottomBar::Side bottomBarSide)
        : HBox(parent, x, y, w, h), bottomBarTemporaryMode(bottomBarTemporaryMode),
          bottomBarRestoreFromTemporaryMode(bottomBarRestoreFromTemporaryMode), bottomBarSide(bottomBarSide)

    {
        setEdges(RectangleEdge::Bottom);
        setAlignment(gui::Alignment(gui::Alignment::Horizontal::Center, gui::Alignment::Vertical::Center));

        image = new Image("small_tick_W_M");
        image->setVisible(false);
        addWidget(image);

        applyCallbacks();
        setFocusItem(image);
    }

    void CheckBox::applyCallbacks()
    {
        focusChangedCallback = [&](Item &item) {
            if (focus) {
                setFocusItem(image);
                if (image->visible) {
                    bottomBarTemporaryMode(utils::translate("common_uncheck"));
                }
                else {
                    bottomBarTemporaryMode(utils::translate("common_check"));
                }
            }
            else {
                setFocusItem(nullptr);
                bottomBarRestoreFromTemporaryMode();
            }
            return true;
        };

        inputCallback = [&](gui::Item &item, const gui::InputEvent &event) {
            if (!event.isShortRelease()) {
                return false;
            }
            if (bottomBarSide == BottomBar::Side::LEFT) {
                if (event.is(gui::KeyCode::KEY_LF)) {
                    image->setVisible(!image->visible);
                    if (image->visible) {
                        bottomBarTemporaryMode(utils::translate("common_uncheck"));
                    }
                    else {
                        bottomBarTemporaryMode(utils::translate("common_check"));
                    }
                    return true;
                }
            }
            else {
                if (event.is(gui::KeyCode::KEY_ENTER)) {
                    image->setVisible(!image->visible);
                    if (image->visible) {
                        bottomBarTemporaryMode(utils::translate("common_uncheck"));
                    }
                    else {
                        bottomBarTemporaryMode(utils::translate("common_check"));
                    }
                    return true;
                }
            }
            return false;
        };
    }

    void CheckBox::setImageVisible(bool state)
    {
        image->setVisible(state);
    }

    bool CheckBox::isChecked()
    {
        return image->visible;
    }

} /* namespace gui */

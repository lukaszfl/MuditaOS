#include "CheckBoxWithLabelItem.hpp"
#include "application-calendar/widgets/CalendarStyle.hpp"
#include "windows/AppWindow.hpp"
#include <ListView.hpp>
#include <Style.hpp>

namespace gui
{

    CheckBoxWithLabelItem::CheckBoxWithLabelItem(app::Application *application,
                                                 const std::string &description,
                                                 bool checkIsOnLeftBarSide)
        : app(application), checkIsOnLeftBarSide(checkIsOnLeftBarSide)
    {
        app = application;
        assert(app != nullptr);

        setMinimumSize(style::window::default_body_width, style::window::calendar::item::checkBox::height);
        setMargins(gui::Margins(style::margins::small, style::window::calendar::item::checkBox::margin_top, 0, 0));
        setEdges(RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);

        hBox = new gui::HBox(this, 0, 0, 0, 0);
        hBox->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);

        if (checkIsOnLeftBarSide) {
            checkBox = new gui::CheckBox(
                hBox,
                0,
                0,
                0,
                0,
                [=](const UTF8 &text) {
                    app->getCurrentWindow()->bottomBarTemporaryMode(text, BottomBar::Side::LEFT, false);
                },
                [=]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); },
                checkIsOnLeftBarSide);
        }
        else {
            checkBox = new gui::CheckBox(
                hBox,
                0,
                0,
                0,
                0,
                [=](const UTF8 &text) {
                    app->getCurrentWindow()->bottomBarTemporaryMode(text, BottomBar::Side::CENTER, false);
                },
                [=]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); },
                checkIsOnLeftBarSide);
        }
        checkBox->setMinimumSize(style::window::calendar::item::checkBox::input_box_label_w,
                                 style::window::calendar::item::checkBox::height);
        checkBox->activeItem = false;

        descriptionLabel = new gui::Label(hBox, 0, 0, 0, 0);
        descriptionLabel->setMinimumSize(style::window::calendar::item::checkBox::description_label_w,
                                         style::window::calendar::item::checkBox::height);
        descriptionLabel->setMargins(gui::Margins(style::margins::very_big, 0, 0, 0));
        descriptionLabel->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);
        descriptionLabel->setAlignment(Alignment(gui::Alignment::Horizontal::Left, gui::Alignment::Vertical::Center));
        descriptionLabel->setFont(style::window::font::medium);
        descriptionLabel->activeItem = false;
        descriptionLabel->setText(description);

        applyCallbacks();
    }

    void CheckBoxWithLabelItem::applyCallbacks()
    {
        focusChangedCallback = [&](Item &item) {
            if (focus) {
                descriptionLabel->setFont(style::window::font::mediumbold);
                setFocusItem(checkBox);
            }
            else {
                descriptionLabel->setFont(style::window::font::medium);
                setFocusItem(nullptr);
            }
            return true;
        };

        inputCallback = [&](gui::Item &item, const gui::InputEvent &event) {
            if (checkBox->onInput(event)) {
                checkBox->resizeItems();
                return true;
            }
            return false;
        };
    }

    bool CheckBoxWithLabelItem::onDimensionChanged(const BoundingBox &oldDim, const BoundingBox &newDim)
    {
        hBox->setPosition(0, 0);
        hBox->setSize(newDim.w, newDim.h);
        return true;
    }

} /* namespace gui */

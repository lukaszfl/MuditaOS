#pragma once
#include "Application.hpp"
#include "CalendarListItem.hpp"
#include "application-calendar/widgets/CalendarStyle.hpp"
#include <module-apps/application-calendar/data/CalendarData.hpp>
#include <Label.hpp>
#include <Image.hpp>
#include <BoxLayout.hpp>

namespace gui
{
    const std::array<uint32_t, 10> reminderTimeOptions = {style::window::calendar::reminder::never,
                                                          style::window::calendar::reminder::event_time,
                                                          style::window::calendar::reminder::five_min_before,
                                                          style::window::calendar::reminder::fifteen_min_before,
                                                          style::window::calendar::reminder::thirty_min_before,
                                                          style::window::calendar::reminder::one_hour_before,
                                                          style::window::calendar::reminder::two_hour_before,
                                                          style::window::calendar::reminder::one_day_before,
                                                          style::window::calendar::reminder::two_days_before,
                                                          style::window::calendar::reminder::one_week_before};

    class SeveralOptionsItem : public CalendarListItem
    {
        app::Application *application = nullptr;
        gui::VBox *vBox               = nullptr;
        gui::HBox *hBox               = nullptr;
        gui::Label *optionLabel       = nullptr;
        gui::Label *descriptionLabel  = nullptr;
        gui::Image *leftArrow         = nullptr;
        gui::Image *rightArrow        = nullptr;
        std::vector<std::string> optionsNames;

        unsigned int actualVectorIndex = 0;

        std::function<void(const UTF8 &text)> bottomBarTemporaryMode = nullptr;
        std::function<void()> bottomBarRestoreFromTemporaryMode      = nullptr;

      public:
        SeveralOptionsItem(app::Application *app,
                           const std::string &description,
                           std::function<void(const UTF8 &text)> bottomBarTemporaryMode = nullptr,
                           std::function<void()> bottomBarRestoreFromTemporaryMode      = nullptr);
        virtual ~SeveralOptionsItem() override = default;

        void prepareOptionsNames();
        void applyCallbacks();
        uint32_t repeatOptionValue;

        // virtual methods from Item
        bool onDimensionChanged(const BoundingBox &oldDim, const BoundingBox &newDim) override;
    };

} /* namespace gui */

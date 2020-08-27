#pragma once
#include "Application.hpp"
#include "CalendarListItem.hpp"
#include <module-apps/application-calendar/data/CalendarData.hpp>
#include <Label.hpp>
#include <Image.hpp>
#include <BoxLayout.hpp>

namespace gui
{
    const std::array<uint32_t, 10> reminderTimeOptions = {0xFFFFFFFF, 0, 5, 15, 30, 100, 200, 10000, 20000, 70000};

    class SeveralOptionsItem : public CalendarListItem
    {
        ////Information to DB DOCUMENTATION
        enum class CustomRepeatOption
        {
            MONDAYS   = 0x10000,
            TUESDAYS  = 0x20000,
            WENSDAYS  = 0x40000,
            THURSDAYS = 0x80000,
            FRIDAYS   = 0x100000,
            SATURDAYS = 0x200000,
            SUNDAYS   = 0x400000
        };

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

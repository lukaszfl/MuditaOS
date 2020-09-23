#pragma once

#include <functional>
#include <string>

#include "AppWindow.hpp"
#include "gui/widgets/Image.hpp"
#include "gui/widgets/Label.hpp"
#include "gui/widgets/Window.hpp"
#include <memory>
#include <BoxLayout.hpp>
#include <service-bluetooth/messages/BluetoothMessage.hpp>

namespace gui
{

    class BtScanWindow : public AppWindow
    {
      protected:
        VBox *box;

        gui::Item *addOptionLabel(const std::string &text, std::function<bool(Item &)> activatedCallback);

      public:
        BtScanWindow(app::Application *app, std::vector<Devicei> devices);

        void onBeforeShow(ShowMode mode, SwitchData *data) override;
        void rebuild() override;
        void buildInterface() override;
        bool onInput(const InputEvent &inputEvent) override;
        void destroyInterface() override;
        void set_navigation();

      private:
        void invalidate() noexcept;
        std::vector<Devicei> devices;
    };
} // namespace gui

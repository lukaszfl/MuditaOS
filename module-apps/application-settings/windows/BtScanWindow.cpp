#include <functional>
#include <memory>

#include "service-appmgr/ApplicationManager.hpp"

#include "../ApplicationSettings.hpp"

#include "i18/i18.hpp"

#include "BtScanWindow.hpp"
#include "Label.hpp"
#include "Margins.hpp"

#include <Style.hpp>

namespace gui
{

    BtScanWindow::BtScanWindow(app::Application *app, std::vector<Devicei> devices)
        : AppWindow(app, "BT_SCAN"), devices(devices)
    {
        buildInterface();
    }

    void BtScanWindow::rebuild()
    {
        destroyInterface();
        buildInterface();
    }

    void add_box_label2(VBox *layout, UTF8 name, std::function<bool(Item &)> foo)
    {
        auto el = new gui::Label(nullptr, 0, 0, layout->getWidth(), style::window::label::default_h);
        style::window::decorateOption(el);
        el->setText(name);
        el->activatedCallback = foo;
        layout->addWidget(el);
    }

    sys::ReturnCodes message_bt2(app::Application *app, BluetoothMessage::Request req)
    {
        std::shared_ptr<BluetoothMessage> msg = std::make_shared<BluetoothMessage>(req);
        auto ret                              = sys::Bus::SendUnicast(msg, "ServiceBluetooth", app, 5000);
        if (ret.first != sys::ReturnCodes::Success) {
            LOG_ERROR("err: %d", static_cast<int>(ret.first));
        }
        return ret.first;
    }

    void BtScanWindow::buildInterface()
    {
        AppWindow::buildInterface();
        bottomBar->setActive(BottomBar::Side::CENTER, true);
        bottomBar->setActive(BottomBar::Side::RIGHT, true);
        bottomBar->setText(BottomBar::Side::CENTER, utils::localize.get(style::strings::common::select));
        bottomBar->setText(BottomBar::Side::RIGHT, utils::localize.get(style::strings::common::back));
        topBar->setActive(TopBar::Elements::SIGNAL, true);
        topBar->setActive(TopBar::Elements::BATTERY, true);

        setTitle(utils::localize.get("BT_scan_results"));

        LOG_INFO("Create box layout, devices count: %u", devices.size());
        box = new gui::VBox(this, 0, title->offset_h(), style::window_width, 7 * style::window::label::default_h);

        //        // TODO WIP: it's just for usability now
        //        // TODO scan should return async - handle that... (if in scan -> add to list and refresh if new on
        //        window) add_box_label2(box, "BT SCAN", [=](Item &) {
        //            LOG_DEBUG("Callback Bluetooth on");
        //            message_bt2(application, BluetoothMessage::Request::Start);
        //            for (auto &el : box->children) {
        //                el->visible = true;
        //            }
        //
        //            return true;
        //        });

        for (auto device : devices) {
            add_box_label2(box, device.name, [=](Item &) {
                LOG_DEBUG("Device: %s", device.name.c_str());

                std::shared_ptr<BluetoothAddrMessage> msg =
                    std::make_shared<BluetoothAddrMessage>(bd_addr_to_str(device.address));
                sys::Bus::SendUnicast(msg, "ServiceBluetooth", application, 5000);
                message_bt2(application, BluetoothMessage::Request::StopScan);
                message_bt2(application, BluetoothMessage::Request::Start);

                return true;
            });
        }
        for (auto &el : box->children) {
            el->visible = true;
        }
        box->setPenWidth(style::window::default_border_no_focus_w);
        box->setVisible(true);
        setFocusItem(box);
        application->render(gui::RefreshModes::GUI_REFRESH_FAST);
    }

    bool BtScanWindow::onInput(const InputEvent &inputEvent)
    {
        return AppWindow::onInput(inputEvent);
    }

    void BtScanWindow::destroyInterface()
    {
        erase();
        invalidate();
    }

    void BtScanWindow::invalidate() noexcept
    {
        box = nullptr;
    }

    void BtScanWindow::onBeforeShow(ShowMode mode, SwitchData *data)
    {}

} // namespace gui

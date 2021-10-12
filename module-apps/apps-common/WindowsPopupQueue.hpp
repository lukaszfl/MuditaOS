// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "popups/data/PopupRequestParams.hpp"
#include "time/time_conversion.hpp"
#include <functional>
#include <set>
#include <utility>

namespace app
{
    class WindowsStack;
}
namespace gui
{
    namespace popup
    {
        class Filter;
        /// Blueprint to create popup window/handle popup action if there is no window when required
        typedef std::function<bool(gui::popup::ID, std::unique_ptr<gui::PopupRequestParams> &)> Blueprint;

        /// sortable class to select next element to show
        class Request
        {
            const gui::popup::ID id;
            std::unique_ptr<gui::PopupRequestParams> params;
            utils::time::Timestamp timeRequested;
            Blueprint blueprint;

          public:
            Request(gui::popup::ID id, std::unique_ptr<gui::PopupRequestParams> &&params, Blueprint blueprint)
                : id(id), params(std::move(params)), blueprint(std::move(blueprint))
            {
                timeRequested = utils::time::getCurrentTimestamp();
            }

            bool handle()
            {
                return blueprint(id, params);
            }

            bool operator<(const Request &p) const
            {
                return params->getDisposition().priority > p.params->getDisposition().priority;
            }

            const gui::PopupRequestParams &getPopupParams() const
            {
                return *params;
            }
        };

    } // namespace popup
} // namespace gui

namespace app
{
    class WindowsPopupQueue
    {
      public:
        /// pops next available popup we require to show
        /// * in order set by Disposition
        /// * limited by Filter passed to request
        std::optional<gui::popup::Request> popRequest(const gui::popup::Filter &filter);

        /// append element to stack
        /// returns success/fail
        bool pushRequest(gui::popup::Request &&r);

      private:
        /// set of requests to handle if there are more than one
        /// requests have to be sortable (this is by design)
        /// to iterate by Requests in priority order
        /// this behaviour can be easily changed - just change:
        /// - gui::popup::Disposition class to hold more data
        /// - change comparison algorithm in Request (please use std::set then)
        std::set<gui::popup::Request> requests{};
    };
} // namespace app

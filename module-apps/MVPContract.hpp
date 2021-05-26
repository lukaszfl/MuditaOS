// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <cassert>

namespace app::mvp
{
    class View;

    /* Polymorphic Presenter base */
    class BasePresenter
    {
      public:
        virtual ~BasePresenter() noexcept = default;

        void attach(View *oth) noexcept
        {
            view = oth;
        }

      protected:
        View *view = nullptr;
    };

    /* Presenter template for a specific View */
    template <typename VIEW> class Presenter : public BasePresenter
    {
      public:
        VIEW *getView() const noexcept
        {
            auto v = dynamic_cast<VIEW *>(view);
            assert(v != nullptr);
            return v;
        }
    };

    /* View always attaches a presenter */
    class View
    {
      public:
        explicit View(BasePresenter *presenter) noexcept
        {
            presenter->attach(this);
        }
        virtual ~View() noexcept = default;
    };

} // namespace app::mvp

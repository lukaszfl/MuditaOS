#pragma once

#include <SwitchData.hpp>
#include <OptionsWindowOption.hpp>
#include <list>

namespace gui
{
    class OptionsWindowOptions : public SwitchData
    {
        std::list<gui::Option> options;

        public:
        OptionsWindowOptions(std::list<gui::Option> options) : options(std::move(options))
        {
        }

        std::list<gui::Option> &&getOptions() 
        {
            return std::move(options);
        }
    };
};


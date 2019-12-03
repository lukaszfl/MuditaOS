/*
 * @file CalculatorSettings.hpp
 * @author Mateusz Grzywacz
 * @date 2 gru 2019
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */
#ifndef MODULE_APPS_APPLICATION_CALCULATOR_APPLICATIONCALCULATOR_HPP_
#define MODULE_APPS_APPLICATION_CALCULATOR_APPLICATIONCALCULATOR_HPP_

#include "Application.hpp"

namespace app
{
namespace name
{
const inline std::string calculator = "ApplicationCalculator";
};

/*
 *
 */
class ApplicationCalculator : public app::Application
{
  public:
    ApplicationCalculator(std::string name = name::calculator, std::string parent = "", bool startBackgound = false);
    virtual ~ApplicationCalculator();
    sys::Message_t DataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp) override;
    sys::ReturnCodes InitHandler() override;
    sys::ReturnCodes DeinitHandler() override;

    sys::ReturnCodes SwitchPowerModeHandler(const sys::ServicePowerMode mode) override final
    {
        return sys::ReturnCodes::Success;
    }

    void createUserInterface();
    void destroyUserInterface();
};

} /* namespace app */

#endif /* MODULE_APPS_APPLICATION_CALCULATOR_APPLICATIONCALCULATOR_HPP_ */

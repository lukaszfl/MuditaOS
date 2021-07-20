// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "SimContactsImportModel.hpp"

#include <application-settings/widgets/network/SimImportContactSelectWidget.hpp>
#include <ListView.hpp>
#include <i18n/i18n.hpp>

SimContactsImportModel::SimContactsImportModel(app::Application *app) : application(app)
{
    createData();
}

auto SimContactsImportModel::requestRecordsCount() -> unsigned int
{
    return internalData.size();
}

auto SimContactsImportModel::getMinimalItemSpaceRequired() const -> unsigned int
{
    return style::window::label::big_h + style::margins::big;
}

void SimContactsImportModel::requestRecords(const uint32_t offset, const uint32_t limit)
{
    setupModel(offset, limit);
    list->onProviderDataUpdate();
}

auto SimContactsImportModel::getItem(gui::Order order) -> gui::ListItem *
{
    return getRecord(order);
}

void SimContactsImportModel::createData()
{
    auto app = application;

    internalData.push_back(new gui::SimImportContactSelectWidget(
        "Przemek!!!!",
        [app](const UTF8 &text) { app->getCurrentWindow()->bottomBarTemporaryMode(text, false); },
        [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); }));

    internalData.push_back(new gui::SimImportContactSelectWidget(
        "Zbyszek!!!!",
        [app](const UTF8 &text) { app->getCurrentWindow()->bottomBarTemporaryMode(text, false); },
        [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); }));

    internalData.push_back(new gui::SimImportContactSelectWidget(
        "Marek!!!!",
        [app](const UTF8 &text) { app->getCurrentWindow()->bottomBarTemporaryMode(text, false); },
        [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); }));

    for (auto item : internalData) {
        item->deleteByList = false;
    }
}

#pragma once

#include "ui/frontend/uifrontend.h"

#include <optional>

namespace mergeqt::ui {

class MainWindow;

class WidgetsFrontend : public UiFrontend
{
public:
    [[nodiscard]] const char *frontendId() const override;
    void openMainWindow(const mergeqt::app::ComparePageDescriptor &initialDescriptor = {}) override;
    int run(QCoreApplication &app,
            std::optional<mergeqt::app::ComparePageDescriptor> initialDescriptor = std::nullopt) override;
};

} // namespace mergeqt::ui

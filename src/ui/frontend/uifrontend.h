#pragma once

#include "app/compare/comparetypes.h"

class QCoreApplication;

namespace mergeqt::ui {

class FrontendWindowHost
{
public:
    virtual ~FrontendWindowHost() = default;
    virtual void openMainWindow(const mergeqt::app::ComparePageDescriptor &initialDescriptor = {}) = 0;
};

class UiFrontend : public FrontendWindowHost
{
public:
    virtual ~UiFrontend() = default;
    [[nodiscard]] virtual const char *frontendId() const = 0;
    virtual int run(QCoreApplication &app,
                    std::optional<mergeqt::app::ComparePageDescriptor> initialDescriptor = std::nullopt) = 0;
};

} // namespace mergeqt::ui

#pragma once

#include "app/compare/comparetypes.h"

#include <optional>
#include <QStringList>

namespace mergeqt::app {

struct ApplicationLaunchRequest
{
    std::optional<mergeqt::app::ComparePageDescriptor> initialCompareDescriptor;
};

[[nodiscard]] ApplicationLaunchRequest parseApplicationLaunchRequest(const QStringList &arguments);

} // namespace mergeqt::app

#pragma once

#include "ui/frontend/uifrontend.h"

#include <memory>

namespace mergeqt::ui {

[[nodiscard]] std::unique_ptr<UiFrontend> createDefaultFrontend();

} // namespace mergeqt::ui

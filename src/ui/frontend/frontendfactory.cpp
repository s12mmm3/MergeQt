#include "ui/frontend/frontendfactory.h"

#include "ui/widgets/widgetsfrontend.h"

#include <memory>

namespace mergeqt::ui {

std::unique_ptr<UiFrontend> createDefaultFrontend()
{
    return std::make_unique<WidgetsFrontend>();
}

} // namespace mergeqt::ui

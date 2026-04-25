#include "ui/widgets/widgetsfrontend.h"

#include "ui/widgets/mainwindow.h"

#include <QCoreApplication>

namespace mergeqt::ui {

const char *WidgetsFrontend::frontendId() const
{
    return "widgets";
}

void WidgetsFrontend::openMainWindow(const mergeqt::app::ComparePageDescriptor &initialDescriptor)
{
    auto *window = new MainWindow(this, initialDescriptor);
    window->show();
}

int WidgetsFrontend::run(QCoreApplication &app,
                         std::optional<mergeqt::app::ComparePageDescriptor> initialDescriptor)
{
    if (initialDescriptor.has_value())
        openMainWindow(*initialDescriptor);
    else
        openMainWindow();
    return app.exec();
}

} // namespace mergeqt::ui

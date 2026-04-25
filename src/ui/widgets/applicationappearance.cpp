#include "ui/widgets/applicationappearance.h"

#include "app/settings/settingsservice.h"

#include <QApplication>
#include <QGuiApplication>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleHints>
#include <QWidget>

namespace mergeqt::ui {

namespace {

QString defaultStyleName()
{
    static const QString styleName = []() {
        auto *application = qobject_cast<QApplication *>(QCoreApplication::instance());
        return application && application->style() ? application->style()->objectName() : QString();
    }();
    return styleName;
}

} // namespace

void applyApplicationAppearance()
{
    auto *application = qobject_cast<QApplication *>(QCoreApplication::instance());
    if (!application)
        return;

    const auto *uiSettings = mergeqt::app::SettingsService::instance()->ui();
    const QString styleName = uiSettings->style().trimmed();
    if (styleName.isEmpty()) {
        if (QStyle *style = QStyleFactory::create(defaultStyleName()))
            application->setStyle(style);
    } else if (QStyle *style = QStyleFactory::create(styleName)) {
        application->setStyle(style);
    }

    Qt::ColorScheme colorScheme = Qt::ColorScheme::Unknown;
    switch (uiSettings->colorScheme()) {
    case 1:
        colorScheme = Qt::ColorScheme::Dark;
        break;
    case 0:
        colorScheme = Qt::ColorScheme::Light;
        break;
    case -1:
    default:
        colorScheme = Qt::ColorScheme::Unknown;
        break;
    }

    QGuiApplication::styleHints()->setColorScheme(colorScheme);

    for (QWidget *widget : application->topLevelWidgets()) {
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
        widget->update();
    }
}

} // namespace mergeqt::ui

#include "app/application/applicationbootstrap.h"

#include "app/application/appcontext.h"
#include "app/settings/settingsservice.h"
#include "platform/path/pathservice.h"

#include <QCoreApplication>

namespace mergeqt::app {

ApplicationLaunchRequest ApplicationBootstrap::initialize(QCoreApplication &app)
{
    mergeqt::platform::PathService::instance();
    auto *settings = SettingsService::instance();
    auto *appContext = AppContext::instance();

    appContext->setSystemLanguage();
    appContext->setStatusMessage(QStringLiteral("MergeQt text compare shell ready"));

    Q_UNUSED(settings);

    return parseApplicationLaunchRequest(app.arguments());
}

} // namespace mergeqt::app

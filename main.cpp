#include "app/application/applicationbootstrap.h"
#include "ui/frontend/frontendfactory.h"
#include "ui/frontend/uifrontend.h"
#include "ui/widgets/applicationappearance.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName(QStringLiteral(PROJECT_NAME));
    QCoreApplication::setOrganizationName(QStringLiteral("MergeQt"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("mergeqt.local"));

    QApplication app(argc, argv);
    const auto launchRequest = mergeqt::app::ApplicationBootstrap::initialize(app);
    mergeqt::ui::applyApplicationAppearance();
    auto frontend = mergeqt::ui::createDefaultFrontend();

    return frontend->run(app, launchRequest.initialCompareDescriptor);
}

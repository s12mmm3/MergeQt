#include "app/application/applicationlaunchrequest.h"

#include <QCommandLineOption>
#include <QCommandLineParser>

namespace mergeqt::app {

namespace {

ComparePageType comparePageTypeFromInput(const QString &typeText)
{
    const QString normalized = typeText.trimmed().toLower();
    if (normalized == QStringLiteral("folder"))
        return ComparePageType::Folder;
    if (normalized == QStringLiteral("binary"))
        return ComparePageType::Binary;
    if (normalized == QStringLiteral("image"))
        return ComparePageType::Image;
    return ComparePageType::Text;
}

} // namespace

ApplicationLaunchRequest parseApplicationLaunchRequest(const QStringList &arguments)
{
    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    QCommandLineOption compareTypeOption(QStringList{ QStringLiteral("compare-type") },
                                         QStringLiteral("Initial compare type."),
                                         QStringLiteral("type"));
    QCommandLineOption leftOption(QStringList{ QStringLiteral("left") },
                                  QStringLiteral("Initial left-side path."),
                                  QStringLiteral("path"));
    QCommandLineOption rightOption(QStringList{ QStringLiteral("right") },
                                   QStringLiteral("Initial right-side path."),
                                   QStringLiteral("path"));
    QCommandLineOption titleOption(QStringList{ QStringLiteral("title") },
                                   QStringLiteral("Initial compare tab title."),
                                   QStringLiteral("title"));

    parser.addOption(compareTypeOption);
    parser.addOption(leftOption);
    parser.addOption(rightOption);
    parser.addOption(titleOption);
    parser.addPositionalArgument(QStringLiteral("paths"), QStringLiteral("Optional initial compare paths."));
    parser.parse(arguments);

    ComparePageDescriptor descriptor;
    descriptor.type = comparePageTypeFromInput(parser.value(compareTypeOption));
    descriptor.leftPath = parser.value(leftOption).trimmed();
    descriptor.rightPath = parser.value(rightOption).trimmed();
    descriptor.title = parser.value(titleOption).trimmed();

    const QStringList positional = parser.positionalArguments();
    if (descriptor.leftPath.isEmpty() && !positional.isEmpty())
        descriptor.leftPath = positional.value(0).trimmed();
    if (descriptor.rightPath.isEmpty() && positional.size() > 1)
        descriptor.rightPath = positional.value(1).trimmed();

    ApplicationLaunchRequest request;
    if (!descriptor.leftPath.isEmpty() || !descriptor.rightPath.isEmpty() || !descriptor.title.isEmpty()
        || parser.isSet(compareTypeOption)) {
        request.initialCompareDescriptor = descriptor;
    }

    return request;
}

} // namespace mergeqt::app

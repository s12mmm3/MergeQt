#include "app/application/applicationlaunchrequest.h"

#include <QtTest>

using namespace mergeqt::app;

class ApplicationLaunchRequestTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void parsesPositionalPathsIntoInitialCompare()
    {
        const auto request = parseApplicationLaunchRequest(
            { QStringLiteral("MergeQt"), QStringLiteral("/tmp/left.txt"), QStringLiteral("/tmp/right.txt") });

        QVERIFY(request.initialCompareDescriptor.has_value());
        QCOMPARE(request.initialCompareDescriptor->type, ComparePageType::Text);
        QCOMPARE(request.initialCompareDescriptor->leftPath, QStringLiteral("/tmp/left.txt"));
        QCOMPARE(request.initialCompareDescriptor->rightPath, QStringLiteral("/tmp/right.txt"));
    }

    void parsesExplicitOptions()
    {
        const auto request = parseApplicationLaunchRequest({
            QStringLiteral("MergeQt"),
            QStringLiteral("--compare-type"),
            QStringLiteral("image"),
            QStringLiteral("--left"),
            QStringLiteral("/tmp/a.png"),
            QStringLiteral("--right"),
            QStringLiteral("/tmp/b.png"),
            QStringLiteral("--title"),
            QStringLiteral("CLI Image Compare"),
        });

        QVERIFY(request.initialCompareDescriptor.has_value());
        QCOMPARE(request.initialCompareDescriptor->type, ComparePageType::Image);
        QCOMPARE(request.initialCompareDescriptor->leftPath, QStringLiteral("/tmp/a.png"));
        QCOMPARE(request.initialCompareDescriptor->rightPath, QStringLiteral("/tmp/b.png"));
        QCOMPARE(request.initialCompareDescriptor->title, QStringLiteral("CLI Image Compare"));
    }

    void returnsEmptyRequestWithoutInitialInputs()
    {
        const auto request = parseApplicationLaunchRequest({ QStringLiteral("MergeQt") });
        QVERIFY(!request.initialCompareDescriptor.has_value());
    }
};

QTEST_MAIN(ApplicationLaunchRequestTest)

#include "app_application_launch_request_test.moc"

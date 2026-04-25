#include "app/compare/imagecomparesessionservice.h"

#include <QImage>
#include <QTemporaryDir>
#include <QtTest>

using namespace mergeqt::app;

class ImageCompareSessionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void reportsIdenticalImages()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        const QString leftPath = dir.filePath(QStringLiteral("left.png"));
        const QString rightPath = dir.filePath(QStringLiteral("right.png"));

        QImage left(4, 4, QImage::Format_ARGB32_Premultiplied);
        left.fill(Qt::red);
        QVERIFY(left.save(leftPath));

        QImage right = left;
        QVERIFY(right.save(rightPath));

        ImageCompareSessionService service;
        QVERIFY(service.compareFiles(leftPath, rightPath));

        const auto result = service.currentResult();
        QVERIFY(result.identicalPixels);
        QVERIFY(result.sameDimensions);
        QCOMPARE(result.differingPixelCount, 0);
    }

    void reportsDifferingPixels()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        const QString leftPath = dir.filePath(QStringLiteral("left.png"));
        const QString rightPath = dir.filePath(QStringLiteral("right.png"));

        QImage left(4, 4, QImage::Format_ARGB32_Premultiplied);
        left.fill(Qt::red);
        QVERIFY(left.save(leftPath));

        QImage right = left;
        right.setPixelColor(2, 1, Qt::blue);
        QVERIFY(right.save(rightPath));

        ImageCompareSessionService service;
        QVERIFY(service.compareFiles(leftPath, rightPath));

        const auto result = service.currentResult();
        QVERIFY(!result.identicalPixels);
        QVERIFY(result.sameDimensions);
        QCOMPARE(result.differingPixelCount, 1);
    }
};

QTEST_MAIN(ImageCompareSessionTest)

#include "app_image_compare_session_test.moc"

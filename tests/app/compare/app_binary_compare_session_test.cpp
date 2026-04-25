#include "app/compare/binarycomparesessionservice.h"

#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

using namespace mergeqt::app;

class BinaryCompareSessionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void reportsIdenticalFiles()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        const QString leftPath = dir.filePath(QStringLiteral("left.bin"));
        const QString rightPath = dir.filePath(QStringLiteral("right.bin"));

        QFile left(leftPath);
        QVERIFY(left.open(QIODevice::WriteOnly));
        QVERIFY(left.write("abcdef", 6) == 6);
        left.close();

        QFile right(rightPath);
        QVERIFY(right.open(QIODevice::WriteOnly));
        QVERIFY(right.write("abcdef", 6) == 6);
        right.close();

        BinaryCompareSessionService service;
        QVERIFY(service.compareFiles(leftPath, rightPath));

        const auto result = service.currentResult();
        QVERIFY(result.identicalContent);
        QVERIFY(result.sameSize);
        QCOMPARE(result.leftSize, 6);
        QCOMPARE(result.rightSize, 6);
    }

    void reportsDifferentContentForSameSizeFiles()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        const QString leftPath = dir.filePath(QStringLiteral("left.bin"));
        const QString rightPath = dir.filePath(QStringLiteral("right.bin"));

        QFile left(leftPath);
        QVERIFY(left.open(QIODevice::WriteOnly));
        QVERIFY(left.write("abcdef", 6) == 6);
        left.close();

        QFile right(rightPath);
        QVERIFY(right.open(QIODevice::WriteOnly));
        QVERIFY(right.write("abcdeg", 6) == 6);
        right.close();

        BinaryCompareSessionService service;
        QVERIFY(service.compareFiles(leftPath, rightPath));

        const auto result = service.currentResult();
        QVERIFY(!result.identicalContent);
        QVERIFY(result.sameSize);
    }
};

QTEST_MAIN(BinaryCompareSessionTest)

#include "app_binary_compare_session_test.moc"

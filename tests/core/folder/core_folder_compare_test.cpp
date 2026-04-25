#include "core/folder/foldercompareengine.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

namespace {

void writeFile(const QString &path, const QByteArray &contents)
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(file.write(contents), contents.size());
}

} // namespace

class FolderCompareEngineTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void comparesRelativePathsAndContent()
    {
        QTemporaryDir leftRoot;
        QTemporaryDir rightRoot;
        QVERIFY(leftRoot.isValid());
        QVERIFY(rightRoot.isValid());

        QDir leftDir(leftRoot.path());
        QDir rightDir(rightRoot.path());
        QVERIFY(leftDir.mkpath(QStringLiteral("nested")));
        QVERIFY(rightDir.mkpath(QStringLiteral("nested")));
        QVERIFY(leftDir.mkpath(QStringLiteral("left-only-dir")));
        QVERIFY(rightDir.mkpath(QStringLiteral("right-only-dir")));

        writeFile(leftDir.filePath(QStringLiteral("same.txt")), "same");
        writeFile(rightDir.filePath(QStringLiteral("same.txt")), "same");
        writeFile(leftDir.filePath(QStringLiteral("changed.txt")), "left");
        writeFile(rightDir.filePath(QStringLiteral("changed.txt")), "right");
        writeFile(leftDir.filePath(QStringLiteral("left-only.txt")), "left-only");
        writeFile(rightDir.filePath(QStringLiteral("right-only.txt")), "right-only");
        writeFile(leftDir.filePath(QStringLiteral("nested/type-entry")), "file");
        QVERIFY(rightDir.mkpath(QStringLiteral("nested/type-entry")));

        mergeqt::core::FolderCompareEngine engine;
        mergeqt::core::FolderCompareResult result;
        QString errorMessage;

        QVERIFY(engine.compare(leftRoot.path(), rightRoot.path(), result, errorMessage));
        QVERIFY(errorMessage.isEmpty());

        QCOMPARE(result.stats.identicalCount, 2);
        QCOMPARE(result.stats.leftOnlyCount, 2);
        QCOMPARE(result.stats.rightOnlyCount, 2);
        QCOMPARE(result.stats.contentMismatchCount, 1);
        QCOMPARE(result.stats.typeMismatchCount, 1);
        QCOMPARE(result.entries.size(), 8);

        const auto findStatus = [&result](const QString &relativePath) {
            for (const auto &entry : result.entries) {
                if (entry.relativePath == relativePath)
                    return entry.status;
            }
            return mergeqt::core::FolderDiffStatus::Identical;
        };

        QCOMPARE(findStatus(QStringLiteral("same.txt")), mergeqt::core::FolderDiffStatus::Identical);
        QCOMPARE(findStatus(QStringLiteral("changed.txt")), mergeqt::core::FolderDiffStatus::ContentMismatch);
        QCOMPARE(findStatus(QStringLiteral("left-only.txt")), mergeqt::core::FolderDiffStatus::LeftOnly);
        QCOMPARE(findStatus(QStringLiteral("right-only.txt")), mergeqt::core::FolderDiffStatus::RightOnly);
        QCOMPARE(findStatus(QStringLiteral("nested/type-entry")), mergeqt::core::FolderDiffStatus::TypeMismatch);
    }

    void reportsMissingFolder()
    {
        mergeqt::core::FolderCompareEngine engine;
        mergeqt::core::FolderCompareResult result;
        QString errorMessage;

        QVERIFY(!engine.compare(QStringLiteral("/definitely/missing/left"),
                                QStringLiteral("/definitely/missing/right"),
                                result,
                                errorMessage));
        QVERIFY(!errorMessage.isEmpty());
    }
};

QTEST_MAIN(FolderCompareEngineTest)

#include "core_folder_compare_test.moc"

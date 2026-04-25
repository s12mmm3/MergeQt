#include "core/folder/folderoperationplanner.h"
#include "core/folder/folderoperationexecutor.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

using namespace mergeqt::core;

class FolderOperationPlannerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void previewsCopyLeftToRightForFiles()
    {
        FolderCompareEntry entry;
        entry.relativePath = QStringLiteral("dir/item.txt");
        entry.leftAbsolutePath = QStringLiteral("/tmp/left/dir/item.txt");
        entry.rightAbsolutePath = QStringLiteral("/tmp/right/dir/item.txt");
        entry.leftKind = FolderEntryKind::File;
        entry.rightKind = FolderEntryKind::Missing;

        FolderOperationPlanner planner;
        const FolderOperationPlan plan = planner.previewPlan(entry, FolderMergeActionKind::CopyLeftToRight);

        QVERIFY(plan.isValid);
        QCOMPARE(plan.steps.size(), 2);
        QCOMPARE(plan.steps.at(0).stepKind, FolderOperationStepKind::EnsureParentDirectory);
        QCOMPARE(plan.steps.at(1).stepKind, FolderOperationStepKind::CopyFile);
        QCOMPARE(plan.steps.at(1).sourcePath, entry.leftAbsolutePath);
        QCOMPARE(plan.steps.at(1).targetPath, entry.rightAbsolutePath);
    }

    void previewsDeleteDirectory()
    {
        FolderCompareEntry entry;
        entry.relativePath = QStringLiteral("orphan-dir");
        entry.leftAbsolutePath = QStringLiteral("/tmp/left/orphan-dir");
        entry.leftKind = FolderEntryKind::Directory;

        FolderOperationPlanner planner;
        const FolderOperationPlan plan = planner.previewPlan(entry, FolderMergeActionKind::DeleteLeft);

        QVERIFY(plan.isValid);
        QCOMPARE(plan.steps.size(), 1);
        QCOMPARE(plan.steps.at(0).stepKind, FolderOperationStepKind::RemoveDirectory);
        QCOMPARE(plan.steps.at(0).sourcePath, entry.leftAbsolutePath);
    }

    void previewsCreateMissingDirectory()
    {
        FolderCompareEntry entry;
        entry.relativePath = QStringLiteral("missing-dir");
        entry.rightAbsolutePath = QStringLiteral("/tmp/right/missing-dir");
        entry.rightKind = FolderEntryKind::Missing;

        FolderOperationPlanner planner;
        const FolderOperationPlan plan = planner.previewPlan(entry, FolderMergeActionKind::CreateRightDirectory);

        QVERIFY(plan.isValid);
        QCOMPARE(plan.steps.size(), 1);
        QCOMPARE(plan.steps.at(0).stepKind, FolderOperationStepKind::CreateDirectory);
        QCOMPARE(plan.steps.at(0).targetPath, entry.rightAbsolutePath);
    }

    void previewsBatchForMatchingEntries()
    {
        FolderCompareEntry leftOnly;
        leftOnly.relativePath = QStringLiteral("left-only.txt");
        leftOnly.leftAbsolutePath = QStringLiteral("/tmp/left/left-only.txt");
        leftOnly.rightAbsolutePath = QStringLiteral("/tmp/right/left-only.txt");
        leftOnly.leftKind = FolderEntryKind::File;
        leftOnly.rightKind = FolderEntryKind::Missing;

        FolderCompareEntry invalidEntry;
        invalidEntry.relativePath = QStringLiteral("invalid.txt");
        invalidEntry.leftKind = FolderEntryKind::File;
        invalidEntry.rightKind = FolderEntryKind::Missing;

        FolderOperationPlanner planner;
        const auto batch = planner.previewBatch({ leftOnly, invalidEntry },
                                                { QStringLiteral("left-only.txt"), QStringLiteral("invalid.txt") },
                                                FolderMergeActionKind::CopyLeftToRight);

        QVERIFY(batch.isValid());
        QCOMPARE(batch.plans.size(), 1);
        QCOMPARE(batch.plans.at(0).relativePath, QStringLiteral("left-only.txt"));
    }

    void executesCreateDirectoryPlan()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        FolderOperationPlan plan;
        plan.isValid = true;
        plan.steps.append({ FolderOperationStepKind::CreateDirectory,
                            {},
                            QDir(tempDir.path()).filePath(QStringLiteral("created/subdir")),
                            {} });

        FolderOperationExecutor executor;
        QString errorMessage;
        QVERIFY(executor.executePlan(plan, errorMessage));
        QVERIFY(errorMessage.isEmpty());
        QVERIFY(QDir(QDir(tempDir.path()).filePath(QStringLiteral("created/subdir"))).exists());
    }

    void executesCopyFilePlan()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const QString sourcePath = QDir(tempDir.path()).filePath(QStringLiteral("source.txt"));
        const QString targetPath = QDir(tempDir.path()).filePath(QStringLiteral("nested/target.txt"));

        QFile sourceFile(sourcePath);
        QVERIFY(sourceFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
        QCOMPARE(sourceFile.write("copy me"), 7LL);
        sourceFile.close();

        FolderOperationPlan plan;
        plan.isValid = true;
        plan.steps.append({ FolderOperationStepKind::EnsureParentDirectory, {}, targetPath, {} });
        plan.steps.append({ FolderOperationStepKind::CopyFile, sourcePath, targetPath, {} });

        FolderOperationExecutor executor;
        QString errorMessage;
        QVERIFY(executor.executePlan(plan, errorMessage));
        QVERIFY(errorMessage.isEmpty());

        QFile targetFile(targetPath);
        QVERIFY(targetFile.open(QIODevice::ReadOnly));
        QCOMPARE(targetFile.readAll(), QByteArray("copy me"));
    }

    void executesBatchUntilFailure()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const QString sourcePath = QDir(tempDir.path()).filePath(QStringLiteral("source.txt"));
        const QString firstTargetPath = QDir(tempDir.path()).filePath(QStringLiteral("first/target.txt"));
        const QString secondTargetPath = QDir(tempDir.path()).filePath(QStringLiteral("second/target.txt"));

        QFile sourceFile(sourcePath);
        QVERIFY(sourceFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
        QCOMPARE(sourceFile.write("batch"), 5LL);
        sourceFile.close();

        FolderOperationPlan validPlan;
        validPlan.isValid = true;
        validPlan.relativePath = QStringLiteral("first/target.txt");
        validPlan.summary = QStringLiteral("Copy left to right");
        validPlan.steps.append({ FolderOperationStepKind::EnsureParentDirectory, {}, firstTargetPath, {} });
        validPlan.steps.append({ FolderOperationStepKind::CopyFile, sourcePath, firstTargetPath, {} });

        FolderOperationPlan invalidPlan;
        invalidPlan.isValid = true;
        invalidPlan.relativePath = QStringLiteral("second/target.txt");
        invalidPlan.summary = QStringLiteral("Copy left to right");
        invalidPlan.steps.append({ FolderOperationStepKind::EnsureParentDirectory, {}, secondTargetPath, {} });
        invalidPlan.steps.append({ FolderOperationStepKind::CopyFile,
                                   QDir(tempDir.path()).filePath(QStringLiteral("missing.txt")),
                                   secondTargetPath,
                                   {} });

        FolderOperationExecutor executor;
        FolderOperationBatch batch;
        batch.plans = { validPlan, invalidPlan };

        const auto report = executor.executeBatch(batch);
        QCOMPARE(report.results.size(), 2);
        QCOMPARE(report.successCount(), 1);
        QCOMPARE(report.failureCount(), 1);

        QFile firstTarget(firstTargetPath);
        QVERIFY(firstTarget.open(QIODevice::ReadOnly));
        QCOMPARE(firstTarget.readAll(), QByteArray("batch"));
        QVERIFY(report.hasFailures());
    }
};

QTEST_MAIN(FolderOperationPlannerTest)

#include "core_folder_operation_planner_test.moc"

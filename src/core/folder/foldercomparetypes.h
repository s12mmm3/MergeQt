#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

namespace mergeqt::core {

enum class FolderEntryKind
{
    Missing,
    File,
    Directory,
    Other
};

enum class FolderDiffStatus
{
    Identical,
    LeftOnly,
    RightOnly,
    ContentMismatch,
    TypeMismatch
};

enum class FolderMergeActionKind
{
    CopyLeftToRight,
    CopyRightToLeft,
    DeleteLeft,
    DeleteRight,
    CreateRightDirectory,
    CreateLeftDirectory
};

enum class FolderOperationStepKind
{
    EnsureParentDirectory,
    CreateDirectory,
    CopyFile,
    DeleteFile,
    RemoveDirectory
};

struct FolderScanEntry
{
    QString absolutePath;
    QString relativePath;
    FolderEntryKind kind = FolderEntryKind::Missing;
    qint64 size = -1;
};

struct FolderCompareEntry
{
    QString relativePath;
    QString leftAbsolutePath;
    QString rightAbsolutePath;
    FolderEntryKind leftKind = FolderEntryKind::Missing;
    FolderEntryKind rightKind = FolderEntryKind::Missing;
    qint64 leftSize = -1;
    qint64 rightSize = -1;
    FolderDiffStatus status = FolderDiffStatus::Identical;

    [[nodiscard]] bool isDifference() const
    {
        return status != FolderDiffStatus::Identical;
    }
};

struct FolderCompareStats
{
    int identicalCount = 0;
    int leftOnlyCount = 0;
    int rightOnlyCount = 0;
    int contentMismatchCount = 0;
    int typeMismatchCount = 0;

    [[nodiscard]] int differenceCount() const
    {
        return leftOnlyCount + rightOnlyCount + contentMismatchCount + typeMismatchCount;
    }
};

struct FolderCompareResult
{
    QString leftRootPath;
    QString rightRootPath;
    QVector<FolderCompareEntry> entries;
    FolderCompareStats stats;

    [[nodiscard]] bool hasDifferences() const
    {
        return stats.differenceCount() > 0;
    }
};

struct FolderOperationPreviewStep
{
    FolderOperationStepKind stepKind = FolderOperationStepKind::CopyFile;
    QString sourcePath;
    QString targetPath;
    QString description;
};

struct FolderOperationPlan
{
    FolderMergeActionKind actionKind = FolderMergeActionKind::CopyLeftToRight;
    QString relativePath;
    QString summary;
    QVector<FolderOperationPreviewStep> steps;
    bool isValid = false;
};

struct FolderOperationBatch
{
    QVector<FolderOperationPlan> plans;
    bool stopOnFailure = true;

    [[nodiscard]] bool isValid() const
    {
        return !plans.isEmpty();
    }

    [[nodiscard]] QStringList relativePaths() const
    {
        QStringList paths;
        paths.reserve(plans.size());
        for (const auto &plan : plans)
            paths.append(plan.relativePath);
        return paths;
    }
};

struct FolderOperationExecutionResult
{
    QString relativePath;
    FolderMergeActionKind actionKind = FolderMergeActionKind::CopyLeftToRight;
    QString summary;
    bool success = false;
    QString message;
};

struct FolderOperationBatchReport
{
    QVector<FolderOperationExecutionResult> results;

    [[nodiscard]] int successCount() const
    {
        int count = 0;
        for (const auto &result : results) {
            if (result.success)
                ++count;
        }
        return count;
    }

    [[nodiscard]] int failureCount() const
    {
        int count = 0;
        for (const auto &result : results) {
            if (!result.success)
                ++count;
        }
        return count;
    }

    [[nodiscard]] bool hasFailures() const
    {
        return failureCount() > 0;
    }

    [[nodiscard]] bool hasSuccesses() const
    {
        return successCount() > 0;
    }
};

} // namespace mergeqt::core

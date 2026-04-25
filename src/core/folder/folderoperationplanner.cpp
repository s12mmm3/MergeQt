#include "core/folder/folderoperationplanner.h"

namespace mergeqt::core {

FolderOperationPlan FolderOperationPlanner::previewPlan(const FolderCompareEntry &entry,
                                                        FolderMergeActionKind actionKind) const
{
    switch (actionKind) {
    case FolderMergeActionKind::CopyLeftToRight:
        return buildCopyLeftToRightPlan(entry);
    case FolderMergeActionKind::CopyRightToLeft:
        return buildCopyRightToLeftPlan(entry);
    case FolderMergeActionKind::DeleteLeft:
        return buildDeleteLeftPlan(entry);
    case FolderMergeActionKind::DeleteRight:
        return buildDeleteRightPlan(entry);
    case FolderMergeActionKind::CreateRightDirectory:
        return buildCreateRightDirectoryPlan(entry);
    case FolderMergeActionKind::CreateLeftDirectory:
        return buildCreateLeftDirectoryPlan(entry);
    }

    return {};
}

FolderOperationBatch FolderOperationPlanner::previewBatch(const QVector<FolderCompareEntry> &entries,
                                                          const QStringList &relativePaths,
                                                          FolderMergeActionKind actionKind) const
{
    FolderOperationBatch batch;
    for (const auto &relativePath : relativePaths) {
        for (const auto &entry : entries) {
            if (entry.relativePath != relativePath)
                continue;

            const auto plan = previewPlan(entry, actionKind);
            if (plan.isValid)
                batch.plans.append(plan);
            break;
        }
    }
    return batch;
}

FolderOperationPlan FolderOperationPlanner::buildCopyLeftToRightPlan(const FolderCompareEntry &entry) const
{
    FolderOperationPlan plan;
    plan.actionKind = FolderMergeActionKind::CopyLeftToRight;
    plan.relativePath = entry.relativePath;
    plan.summary = QStringLiteral("Copy left to right");
    if (entry.leftAbsolutePath.isEmpty() || entry.rightAbsolutePath.isEmpty())
        return plan;

    plan.isValid = true;
    plan.steps.append(makeStep(FolderOperationStepKind::EnsureParentDirectory,
                               {},
                               entry.rightAbsolutePath,
                               QStringLiteral("Ensure the target parent directory exists.")));
    plan.steps.append(makeStep(FolderOperationStepKind::CopyFile,
                               entry.leftAbsolutePath,
                               entry.rightAbsolutePath,
                               QStringLiteral("Copy the left-side file to the right side.")));
    return plan;
}

FolderOperationPlan FolderOperationPlanner::buildCopyRightToLeftPlan(const FolderCompareEntry &entry) const
{
    FolderOperationPlan plan;
    plan.actionKind = FolderMergeActionKind::CopyRightToLeft;
    plan.relativePath = entry.relativePath;
    plan.summary = QStringLiteral("Copy right to left");
    if (entry.leftAbsolutePath.isEmpty() || entry.rightAbsolutePath.isEmpty())
        return plan;

    plan.isValid = true;
    plan.steps.append(makeStep(FolderOperationStepKind::EnsureParentDirectory,
                               {},
                               entry.leftAbsolutePath,
                               QStringLiteral("Ensure the target parent directory exists.")));
    plan.steps.append(makeStep(FolderOperationStepKind::CopyFile,
                               entry.rightAbsolutePath,
                               entry.leftAbsolutePath,
                               QStringLiteral("Copy the right-side file to the left side.")));
    return plan;
}

FolderOperationPlan FolderOperationPlanner::buildDeleteLeftPlan(const FolderCompareEntry &entry) const
{
    FolderOperationPlan plan;
    plan.actionKind = FolderMergeActionKind::DeleteLeft;
    plan.relativePath = entry.relativePath;
    plan.summary = QStringLiteral("Delete left item");
    if (entry.leftAbsolutePath.isEmpty())
        return plan;

    plan.isValid = true;
    const FolderOperationStepKind stepKind =
        entry.leftKind == FolderEntryKind::Directory ? FolderOperationStepKind::RemoveDirectory : FolderOperationStepKind::DeleteFile;
    plan.steps.append(makeStep(stepKind,
                               entry.leftAbsolutePath,
                               {},
                               QStringLiteral("Remove the left-side item.")));
    return plan;
}

FolderOperationPlan FolderOperationPlanner::buildDeleteRightPlan(const FolderCompareEntry &entry) const
{
    FolderOperationPlan plan;
    plan.actionKind = FolderMergeActionKind::DeleteRight;
    plan.relativePath = entry.relativePath;
    plan.summary = QStringLiteral("Delete right item");
    if (entry.rightAbsolutePath.isEmpty())
        return plan;

    plan.isValid = true;
    const FolderOperationStepKind stepKind =
        entry.rightKind == FolderEntryKind::Directory ? FolderOperationStepKind::RemoveDirectory : FolderOperationStepKind::DeleteFile;
    plan.steps.append(makeStep(stepKind,
                               entry.rightAbsolutePath,
                               {},
                               QStringLiteral("Remove the right-side item.")));
    return plan;
}

FolderOperationPlan FolderOperationPlanner::buildCreateRightDirectoryPlan(const FolderCompareEntry &entry) const
{
    FolderOperationPlan plan;
    plan.actionKind = FolderMergeActionKind::CreateRightDirectory;
    plan.relativePath = entry.relativePath;
    plan.summary = QStringLiteral("Create right directory");
    if (entry.rightAbsolutePath.isEmpty())
        return plan;

    plan.isValid = true;
    plan.steps.append(makeStep(FolderOperationStepKind::CreateDirectory,
                               {},
                               entry.rightAbsolutePath,
                               QStringLiteral("Create the missing right-side directory.")));
    return plan;
}

FolderOperationPlan FolderOperationPlanner::buildCreateLeftDirectoryPlan(const FolderCompareEntry &entry) const
{
    FolderOperationPlan plan;
    plan.actionKind = FolderMergeActionKind::CreateLeftDirectory;
    plan.relativePath = entry.relativePath;
    plan.summary = QStringLiteral("Create left directory");
    if (entry.leftAbsolutePath.isEmpty())
        return plan;

    plan.isValid = true;
    plan.steps.append(makeStep(FolderOperationStepKind::CreateDirectory,
                               {},
                               entry.leftAbsolutePath,
                               QStringLiteral("Create the missing left-side directory.")));
    return plan;
}

FolderOperationPreviewStep FolderOperationPlanner::makeStep(FolderOperationStepKind stepKind,
                                                            const QString &sourcePath,
                                                            const QString &targetPath,
                                                            const QString &description) const
{
    FolderOperationPreviewStep step;
    step.stepKind = stepKind;
    step.sourcePath = sourcePath;
    step.targetPath = targetPath;
    step.description = description;
    return step;
}

} // namespace mergeqt::core

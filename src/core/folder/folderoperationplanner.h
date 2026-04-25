#pragma once

#include "core/folder/foldercomparetypes.h"

namespace mergeqt::core {

class FolderOperationPlanner
{
public:
    [[nodiscard]] FolderOperationPlan previewPlan(const FolderCompareEntry &entry,
                                                  FolderMergeActionKind actionKind) const;
    [[nodiscard]] FolderOperationBatch previewBatch(const QVector<FolderCompareEntry> &entries,
                                                    const QStringList &relativePaths,
                                                    FolderMergeActionKind actionKind) const;

private:
    [[nodiscard]] FolderOperationPlan buildCopyLeftToRightPlan(const FolderCompareEntry &entry) const;
    [[nodiscard]] FolderOperationPlan buildCopyRightToLeftPlan(const FolderCompareEntry &entry) const;
    [[nodiscard]] FolderOperationPlan buildDeleteLeftPlan(const FolderCompareEntry &entry) const;
    [[nodiscard]] FolderOperationPlan buildDeleteRightPlan(const FolderCompareEntry &entry) const;
    [[nodiscard]] FolderOperationPlan buildCreateRightDirectoryPlan(const FolderCompareEntry &entry) const;
    [[nodiscard]] FolderOperationPlan buildCreateLeftDirectoryPlan(const FolderCompareEntry &entry) const;
    [[nodiscard]] FolderOperationPreviewStep makeStep(FolderOperationStepKind stepKind,
                                                      const QString &sourcePath,
                                                      const QString &targetPath,
                                                      const QString &description) const;
};

} // namespace mergeqt::core

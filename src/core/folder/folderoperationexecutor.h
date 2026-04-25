#pragma once

#include "core/folder/foldercomparetypes.h"

#include <QString>

namespace mergeqt::core {

class FolderOperationExecutor
{
public:
    [[nodiscard]] bool executePlan(const FolderOperationPlan &plan, QString &errorMessage) const;
    [[nodiscard]] FolderOperationBatchReport executeBatch(const FolderOperationBatch &batch) const;

private:
    [[nodiscard]] bool ensureParentDirectory(const QString &targetPath, QString &errorMessage) const;
    [[nodiscard]] bool createDirectory(const QString &targetPath, QString &errorMessage) const;
    [[nodiscard]] bool copyFile(const QString &sourcePath, const QString &targetPath, QString &errorMessage) const;
};

} // namespace mergeqt::core

#include "core/folder/folderoperationexecutor.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace mergeqt::core {

bool FolderOperationExecutor::executePlan(const FolderOperationPlan &plan, QString &errorMessage) const
{
    if (!plan.isValid || plan.steps.isEmpty()) {
        errorMessage = QObject::tr("The selected operation plan is empty.");
        return false;
    }

    for (const auto &step : plan.steps) {
        bool ok = false;
        switch (step.stepKind) {
        case FolderOperationStepKind::EnsureParentDirectory:
            ok = ensureParentDirectory(step.targetPath, errorMessage);
            break;
        case FolderOperationStepKind::CreateDirectory:
            ok = createDirectory(step.targetPath, errorMessage);
            break;
        case FolderOperationStepKind::CopyFile:
            ok = copyFile(step.sourcePath, step.targetPath, errorMessage);
            break;
        case FolderOperationStepKind::DeleteFile:
        case FolderOperationStepKind::RemoveDirectory:
            errorMessage = QObject::tr("Delete operations are not enabled yet.");
            ok = false;
            break;
        }

        if (!ok)
            return false;
    }

    return true;
}

FolderOperationBatchReport FolderOperationExecutor::executeBatch(const FolderOperationBatch &batch) const
{
    FolderOperationBatchReport report;
    for (const auto &plan : batch.plans) {
        FolderOperationExecutionResult result;
        result.relativePath = plan.relativePath;
        result.actionKind = plan.actionKind;
        result.summary = plan.summary;

        QString errorMessage;
        result.success = executePlan(plan, errorMessage);
        result.message = result.success ? QObject::tr("Executed %1 for %2.").arg(plan.summary, plan.relativePath) : errorMessage;
        report.results.append(result);

        if (!result.success && batch.stopOnFailure)
            break;
    }
    return report;
}

bool FolderOperationExecutor::ensureParentDirectory(const QString &targetPath, QString &errorMessage) const
{
    const QFileInfo info(targetPath);
    const QString parentPath = info.absolutePath();
    if (parentPath.isEmpty()) {
        errorMessage = QObject::tr("Cannot determine the parent directory for: %1").arg(targetPath);
        return false;
    }

    QDir dir(parentPath);
    if (dir.exists())
        return true;

    if (!dir.mkpath(QStringLiteral("."))) {
        errorMessage = QObject::tr("Failed to create the parent directory: %1").arg(parentPath);
        return false;
    }

    return true;
}

bool FolderOperationExecutor::createDirectory(const QString &targetPath, QString &errorMessage) const
{
    if (targetPath.isEmpty()) {
        errorMessage = QObject::tr("The target directory path is empty.");
        return false;
    }

    QDir dir(targetPath);
    if (dir.exists())
        return true;

    if (!dir.mkpath(QStringLiteral("."))) {
        errorMessage = QObject::tr("Failed to create the directory: %1").arg(targetPath);
        return false;
    }

    return true;
}

bool FolderOperationExecutor::copyFile(const QString &sourcePath, const QString &targetPath, QString &errorMessage) const
{
    if (sourcePath.isEmpty() || targetPath.isEmpty()) {
        errorMessage = QObject::tr("The source or target file path is empty.");
        return false;
    }

    QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists() || !sourceInfo.isFile()) {
        errorMessage = QObject::tr("The source file does not exist: %1").arg(sourcePath);
        return false;
    }

    if (!ensureParentDirectory(targetPath, errorMessage))
        return false;

    if (QFile::exists(targetPath) && !QFile::remove(targetPath)) {
        errorMessage = QObject::tr("Failed to overwrite the target file: %1").arg(targetPath);
        return false;
    }

    if (!QFile::copy(sourcePath, targetPath)) {
        errorMessage = QObject::tr("Failed to copy %1 to %2").arg(sourcePath, targetPath);
        return false;
    }

    return true;
}

} // namespace mergeqt::core

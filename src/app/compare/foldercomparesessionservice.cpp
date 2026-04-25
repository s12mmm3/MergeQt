#include "app/compare/foldercomparesessionservice.h"

#include "app/logging/logger.h"
#include "app/settings/settingsservice.h"

namespace mergeqt::app {

FolderCompareSessionService::FolderCompareSessionService(QObject *parent)
    : QObject(parent)
{
}

mergeqt::core::FolderCompareResult FolderCompareSessionService::currentResult() const
{
    return m_currentResult;
}

QString FolderCompareSessionService::currentLeftRootPath() const
{
    return m_currentResult.leftRootPath;
}

QString FolderCompareSessionService::currentRightRootPath() const
{
    return m_currentResult.rightRootPath;
}

mergeqt::core::FolderOperationBatchReport FolderCompareSessionService::lastBatchReport() const
{
    return m_lastBatchReport;
}

mergeqt::core::FolderOperationPlan FolderCompareSessionService::previewOperation(
    const QString &relativePath,
    mergeqt::core::FolderMergeActionKind actionKind) const
{
    for (const auto &entry : m_currentResult.entries) {
        if (entry.relativePath == relativePath)
            return m_operationPlanner.previewPlan(entry, actionKind);
    }

    return {};
}

mergeqt::core::FolderOperationBatch FolderCompareSessionService::previewOperations(
    const QStringList &relativePaths,
    mergeqt::core::FolderMergeActionKind actionKind) const
{
    return m_operationPlanner.previewBatch(m_currentResult.entries, relativePaths, actionKind);
}

bool FolderCompareSessionService::executeOperation(const QString &relativePath, mergeqt::core::FolderMergeActionKind actionKind)
{
    const auto plan = previewOperation(relativePath, actionKind);
    QString errorMessage;
    if (!m_operationExecutor.executePlan(plan, errorMessage)) {
        emit operationFinished(plan.relativePath.isEmpty() ? relativePath : plan.relativePath,
                               actionKind,
                               false,
                               errorMessage);
        emit comparisonFailed(errorMessage);
        emit comparisonStatusChanged(errorMessage);
        return false;
    }

    if (!compareFolders(m_currentResult.leftRootPath, m_currentResult.rightRootPath))
        return false;

    emit operationFinished(plan.relativePath,
                           actionKind,
                           true,
                           tr("Applied %1 to %2.").arg(plan.summary, plan.relativePath));
    return true;
}

bool FolderCompareSessionService::executeOperations(const QStringList &relativePaths,
                                                    mergeqt::core::FolderMergeActionKind actionKind)
{
    const auto batch = previewOperations(relativePaths, actionKind);
    m_lastBatchReport = m_operationExecutor.executeBatch(batch);
    if (m_lastBatchReport.results.isEmpty()) {
        const QString message = tr("No valid operations were available to execute.");
        emit comparisonFailed(message);
        emit comparisonStatusChanged(message);
        emit operationBatchFinished(0, 0, 0, message);
        return false;
    }

    for (const auto &result : m_lastBatchReport.results) {
        emit operationFinished(result.relativePath, result.actionKind, result.success, result.message);
        if (!result.success) {
            emit comparisonFailed(result.message);
            emit comparisonStatusChanged(result.message);
        }
    }

    if (m_lastBatchReport.hasSuccesses())
        compareFolders(m_currentResult.leftRootPath, m_currentResult.rightRootPath);

    const QString message = tr("Executed %1 operations: %2 succeeded, %3 failed.")
                                .arg(m_lastBatchReport.results.size())
                                .arg(m_lastBatchReport.successCount())
                                .arg(m_lastBatchReport.failureCount());
    LOG_INFO << message;
    emit operationBatchFinished(m_lastBatchReport.results.size(),
                                m_lastBatchReport.successCount(),
                                m_lastBatchReport.failureCount(),
                                message);
    emit comparisonStatusChanged(message);
    return !m_lastBatchReport.hasFailures();
}

bool FolderCompareSessionService::compareFolders(const QString &leftRootPath, const QString &rightRootPath)
{
    QString errorMessage;
    if (!m_engine.compare(leftRootPath, rightRootPath, m_currentResult, errorMessage)) {
        emit comparisonFailed(errorMessage);
        emit comparisonStatusChanged(errorMessage);
        return false;
    }

    auto *runtime = SettingsService::instance()->runtime();
    runtime->set_lastLeftFolderPath(m_currentResult.leftRootPath);
    runtime->set_lastRightFolderPath(m_currentResult.rightRootPath);

    const QString message =
        tr("Compared %1 entries: %2 identical, %3 left-only, %4 right-only, %5 content mismatches, %6 type mismatches.")
            .arg(m_currentResult.entries.size())
            .arg(m_currentResult.stats.identicalCount)
            .arg(m_currentResult.stats.leftOnlyCount)
            .arg(m_currentResult.stats.rightOnlyCount)
            .arg(m_currentResult.stats.contentMismatchCount)
            .arg(m_currentResult.stats.typeMismatchCount);
    LOG_INFO << message;
    emit comparisonStatusChanged(message);
    emit comparisonCompleted();
    return true;
}

bool FolderCompareSessionService::reloadCurrentFolders()
{
    if (m_currentResult.leftRootPath.isEmpty() || m_currentResult.rightRootPath.isEmpty()) {
        const QString message = tr("Reload is only available for folder-based comparisons.");
        emit comparisonFailed(message);
        emit comparisonStatusChanged(message);
        return false;
    }

    return compareFolders(m_currentResult.leftRootPath, m_currentResult.rightRootPath);
}

bool FolderCompareSessionService::swapInputs()
{
    return compareFolders(m_currentResult.rightRootPath, m_currentResult.leftRootPath);
}

} // namespace mergeqt::app

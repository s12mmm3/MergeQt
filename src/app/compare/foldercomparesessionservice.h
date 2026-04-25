#pragma once

#include "core/folder/foldercompareengine.h"
#include "core/folder/folderoperationexecutor.h"
#include "core/folder/folderoperationplanner.h"

#include <QObject>

namespace mergeqt::app {

class FolderCompareSessionService : public QObject
{
    Q_OBJECT

public:
    explicit FolderCompareSessionService(QObject *parent = nullptr);

    [[nodiscard]] mergeqt::core::FolderCompareResult currentResult() const;
    [[nodiscard]] QString currentLeftRootPath() const;
    [[nodiscard]] QString currentRightRootPath() const;
    [[nodiscard]] mergeqt::core::FolderOperationBatchReport lastBatchReport() const;
    [[nodiscard]] mergeqt::core::FolderOperationPlan previewOperation(const QString &relativePath,
                                                                      mergeqt::core::FolderMergeActionKind actionKind) const;
    [[nodiscard]] mergeqt::core::FolderOperationBatch previewOperations(const QStringList &relativePaths,
                                                                        mergeqt::core::FolderMergeActionKind actionKind) const;

    Q_INVOKABLE bool compareFolders(const QString &leftRootPath, const QString &rightRootPath);
    Q_INVOKABLE bool executeOperation(const QString &relativePath, mergeqt::core::FolderMergeActionKind actionKind);
    Q_INVOKABLE bool executeOperations(const QStringList &relativePaths, mergeqt::core::FolderMergeActionKind actionKind);
    Q_INVOKABLE bool reloadCurrentFolders();
    Q_INVOKABLE bool swapInputs();

Q_SIGNALS:
    void comparisonCompleted();
    void comparisonFailed(const QString &message);
    void comparisonStatusChanged(const QString &message);
    void operationFinished(const QString &relativePath,
                           mergeqt::core::FolderMergeActionKind actionKind,
                           bool success,
                           const QString &message);
    void operationBatchFinished(int totalCount, int successCount, int failureCount, const QString &message);

private:
    mergeqt::core::FolderCompareEngine m_engine;
    mergeqt::core::FolderOperationExecutor m_operationExecutor;
    mergeqt::core::FolderOperationPlanner m_operationPlanner;
    mergeqt::core::FolderCompareResult m_currentResult;
    mergeqt::core::FolderOperationBatchReport m_lastBatchReport;
};

} // namespace mergeqt::app

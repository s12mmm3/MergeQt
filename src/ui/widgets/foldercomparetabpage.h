#pragma once

#include "app/compare/comparetypes.h"
#include "app/compare/foldercomparesessionservice.h"
#include "ui/widgets/comparetabpage.h"

class QAction;
class QComboBox;
class QLabel;
class QMenu;
class QMessageBox;
class QTableWidget;
class QTableWidgetItem;
class QLineEdit;
class QString;

namespace mergeqt::ui {

class CompareInputHeader;

class FolderCompareTabPage : public CompareTabPage
{
    Q_OBJECT

public:
    explicit FolderCompareTabPage(const mergeqt::app::ComparePageDescriptor &descriptor = {}, QWidget *parent = nullptr);

    [[nodiscard]] mergeqt::app::ComparePageType pageType() const override;
    [[nodiscard]] QString pageTitle() const override;
    [[nodiscard]] QString pageStatusText() const override;
    [[nodiscard]] CompareTabCapabilities capabilities() const override;

    bool chooseLeftFile() override;
    bool chooseRightFile() override;
    bool reloadInputs() override;
    bool swapInputs() override;
    bool executeCompare() override;
    void navigateDifferenceByOffset(int offset) override;
    void applyWindowSettings() override;

private:
    void buildUi();
    void connectSignals();
    void loadStartupState();
    void refreshResultView();
    void selectDifferenceRow(int row);
    QTableWidgetItem *currentStatusItem() const;
    QStringList selectedRelativePaths() const;
    QString currentRelativePath() const;
    QString currentLeftAbsolutePath() const;
    QString currentRightAbsolutePath() const;
    QString currentLeftFileName() const;
    QString currentRightFileName() const;
    QString currentLeftParentPath() const;
    QString currentRightParentPath() const;
    mergeqt::core::FolderOperationPlan previewCurrentOperation(mergeqt::core::FolderMergeActionKind actionKind) const;
    mergeqt::core::FolderOperationBatch previewSelectedOperations(mergeqt::core::FolderMergeActionKind actionKind) const;
    bool currentLeftIsFile() const;
    bool currentRightIsFile() const;
    bool hasSelectedRows() const;
    bool canOpenSelectedEntryInTextCompare() const;
    bool canOpenSelectedLeftOnlyInTextCompare() const;
    bool canOpenSelectedRightOnlyInTextCompare() const;
    bool canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind actionKind) const;
    void openSelectedEntryInTextCompare();
    void openSelectedLeftOnlyInTextCompare();
    void openSelectedRightOnlyInTextCompare();
    void copyRelativePath();
    void copyLeftFileName();
    void copyRightFileName();
    void copyLeftAbsolutePath();
    void copyRightAbsolutePath();
    void copyLeftParentPath();
    void copyRightParentPath();
    void revealLeftInFileManager();
    void revealRightInFileManager();
    void revealLeftParentInFileManager();
    void revealRightParentInFileManager();
    void showMergeActionPlaceholder();
    void previewCopyLeftToRight();
    void previewCopyRightToLeft();
    void previewDeleteLeft();
    void previewDeleteRight();
    void previewCreateLeftDirectory();
    void previewCreateRightDirectory();
    void executeCopyLeftToRight();
    void executeCopyRightToLeft();
    void executeCreateLeftDirectory();
    void executeCreateRightDirectory();
    void showResultContextMenu(const QPoint &position);
    QString entryKindText(mergeqt::core::FolderEntryKind kind) const;
    QString diffStatusText(mergeqt::core::FolderDiffStatus status) const;
    QString sizeText(qint64 size) const;
    QString previewStepKindText(mergeqt::core::FolderOperationStepKind stepKind) const;
    void setStatusMessage(const QString &message);
    bool copyTextToClipboard(const QString &text, const QString &successMessage, const QString &failureMessage);
    bool openPathInFileManager(const QString &path, const QString &successMessage, const QString &failureMessage);
    void showOperationPreview(const mergeqt::core::FolderOperationPlan &plan, const QString &emptyMessage);
    void showOperationBatchPreview(const mergeqt::core::FolderOperationBatch &batch, const QString &emptyMessage);
    bool confirmAndExecuteOperation(const mergeqt::core::FolderOperationPlan &plan, const QString &title);
    bool confirmAndExecuteBatch(const mergeqt::core::FolderOperationBatch &batch, const QString &title);
    void exportReport();
    [[nodiscard]] QVector<mergeqt::core::FolderCompareEntry> filteredEntries() const;
    [[nodiscard]] bool entryMatchesFilter(const mergeqt::core::FolderCompareEntry &entry) const;
    void setOperationMessage(const QString &message, bool success);
    void emitPageState();

    mergeqt::app::FolderCompareSessionService m_compareSession;
    mergeqt::app::ComparePageDescriptor m_descriptor;
    CompareInputHeader *m_inputHeader = nullptr;
    QLineEdit *m_filterEdit = nullptr;
    QComboBox *m_filterModeCombo = nullptr;
    QTableWidget *m_resultTable = nullptr;
    QLabel *m_resultSummaryLabel = nullptr;
    QLabel *m_operationPreviewLabel = nullptr;
    QLabel *m_lastOperationLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QAction *m_showIdenticalItemsAction = nullptr;
    QAction *m_openSelectedTextCompareAction = nullptr;
    QAction *m_openSelectedLeftOnlyTextCompareAction = nullptr;
    QAction *m_openSelectedRightOnlyTextCompareAction = nullptr;
    QAction *m_copyRelativePathAction = nullptr;
    QAction *m_copyLeftFileNameAction = nullptr;
    QAction *m_copyRightFileNameAction = nullptr;
    QAction *m_copyLeftAbsolutePathAction = nullptr;
    QAction *m_copyRightAbsolutePathAction = nullptr;
    QAction *m_copyLeftParentPathAction = nullptr;
    QAction *m_copyRightParentPathAction = nullptr;
    QAction *m_revealLeftInFileManagerAction = nullptr;
    QAction *m_revealRightInFileManagerAction = nullptr;
    QAction *m_revealLeftParentInFileManagerAction = nullptr;
    QAction *m_revealRightParentInFileManagerAction = nullptr;
    QAction *m_mergeActionPlaceholder = nullptr;
    QAction *m_previewCopyLeftToRightAction = nullptr;
    QAction *m_previewCopyRightToLeftAction = nullptr;
    QAction *m_previewDeleteLeftAction = nullptr;
    QAction *m_previewDeleteRightAction = nullptr;
    QAction *m_previewCreateLeftDirectoryAction = nullptr;
    QAction *m_previewCreateRightDirectoryAction = nullptr;
    QAction *m_executeCopyLeftToRightAction = nullptr;
    QAction *m_executeCopyRightToLeftAction = nullptr;
    QAction *m_executeCreateLeftDirectoryAction = nullptr;
    QAction *m_executeCreateRightDirectoryAction = nullptr;
    QAction *m_exportReportAction = nullptr;
};

} // namespace mergeqt::ui

#include "ui/widgets/foldercomparetabpage.h"

#include "app/compare/foldercomparereportformatter.h"
#include "app/settings/settingsservice.h"
#include "ui/widgets/compareinputheader.h"

#include <QAction>
#include <QAbstractItemView>
#include <QComboBox>
#include <QClipboard>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStringList>
#include <QTableWidget>
#include <QUrl>
#include <QVBoxLayout>

namespace mergeqt::ui {

FolderCompareTabPage::FolderCompareTabPage(const mergeqt::app::ComparePageDescriptor &descriptor, QWidget *parent)
    : CompareTabPage(parent)
    , m_compareSession(this)
    , m_descriptor(descriptor)
{
    buildUi();
    connectSignals();
    loadStartupState();
}

mergeqt::app::ComparePageType FolderCompareTabPage::pageType() const
{
    return mergeqt::app::ComparePageType::Folder;
}

QString FolderCompareTabPage::pageTitle() const
{
    const QString left = m_compareSession.currentLeftRootPath().isEmpty()
                             ? tr("Left Folder")
                             : QFileInfo(m_compareSession.currentLeftRootPath()).fileName();
    const QString right = m_compareSession.currentRightRootPath().isEmpty()
                              ? tr("Right Folder")
                              : QFileInfo(m_compareSession.currentRightRootPath()).fileName();
    return tr("%1 vs %2").arg(left, right);
}

QString FolderCompareTabPage::pageStatusText() const
{
    return m_statusLabel ? m_statusLabel->text() : QString();
}

CompareTabCapabilities FolderCompareTabPage::capabilities() const
{
    return { true, true, true, true, true };
}

bool FolderCompareTabPage::chooseLeftFile()
{
    const QString path = QFileDialog::getExistingDirectory(this, tr("Open left folder"), m_inputHeader->leftPathEdit()->text());
    if (path.isEmpty())
        return false;

    m_inputHeader->leftPathEdit()->setText(path);
    return executeCompare();
}

bool FolderCompareTabPage::chooseRightFile()
{
    const QString path = QFileDialog::getExistingDirectory(this, tr("Open right folder"), m_inputHeader->rightPathEdit()->text());
    if (path.isEmpty())
        return false;

    m_inputHeader->rightPathEdit()->setText(path);
    return executeCompare();
}

bool FolderCompareTabPage::reloadInputs()
{
    const bool ok = m_compareSession.reloadCurrentFolders();
    if (ok)
        emitPageState();
    return ok;
}

bool FolderCompareTabPage::swapInputs()
{
    if (!m_compareSession.swapInputs())
        return false;

    m_inputHeader->leftPathEdit()->setText(m_compareSession.currentLeftRootPath());
    m_inputHeader->rightPathEdit()->setText(m_compareSession.currentRightRootPath());
    emitPageState();
    return true;
}

bool FolderCompareTabPage::executeCompare()
{
    const QString leftPath = m_inputHeader->leftPathEdit()->text().trimmed();
    const QString rightPath = m_inputHeader->rightPathEdit()->text().trimmed();
    if (leftPath.isEmpty() || rightPath.isEmpty()) {
        const QString message = tr("Choose two folders before running folder compare.");
        m_statusLabel->setText(message);
        emit pageStatusChanged(message);
        return false;
    }

    if (!m_compareSession.compareFolders(leftPath, rightPath))
        return false;

    m_inputHeader->leftPathEdit()->setText(m_compareSession.currentLeftRootPath());
    m_inputHeader->rightPathEdit()->setText(m_compareSession.currentRightRootPath());
    emitPageState();
    return true;
}

void FolderCompareTabPage::navigateDifferenceByOffset(int offset)
{
    if (!m_resultTable || m_resultTable->rowCount() <= 0)
        return;

    int row = m_resultTable->currentRow();
    if (row < 0)
        row = 0;

    const int rowCount = m_resultTable->rowCount();
    for (int step = 0; step < rowCount; ++step) {
        row = (row + offset + rowCount) % rowCount;
        const auto *item = m_resultTable->item(row, 0);
        if (item && item->data(Qt::UserRole).toBool()) {
            selectDifferenceRow(row);
            return;
        }
    }
}

void FolderCompareTabPage::applyWindowSettings()
{
    const auto *compare = mergeqt::app::SettingsService::instance()->compare();
    QSignalBlocker blocker(m_showIdenticalItemsAction);
    m_showIdenticalItemsAction->setChecked(compare->showIdenticalItems());
    refreshResultView();
}

void FolderCompareTabPage::buildUi()
{
    m_showIdenticalItemsAction = new QAction(tr("Show Identical Items"), this);
    m_showIdenticalItemsAction->setCheckable(true);
    m_openSelectedTextCompareAction = new QAction(tr("Open Selected File Pair"), this);
    m_openSelectedLeftOnlyTextCompareAction = new QAction(tr("Open Left File Against Empty"), this);
    m_openSelectedRightOnlyTextCompareAction = new QAction(tr("Open Right File Against Empty"), this);
    m_copyRelativePathAction = new QAction(tr("Copy Relative Path"), this);
    m_copyLeftFileNameAction = new QAction(tr("Copy Left File Name"), this);
    m_copyRightFileNameAction = new QAction(tr("Copy Right File Name"), this);
    m_copyLeftAbsolutePathAction = new QAction(tr("Copy Left Path"), this);
    m_copyRightAbsolutePathAction = new QAction(tr("Copy Right Path"), this);
    m_copyLeftParentPathAction = new QAction(tr("Copy Left Parent Path"), this);
    m_copyRightParentPathAction = new QAction(tr("Copy Right Parent Path"), this);
    m_revealLeftInFileManagerAction = new QAction(tr("Show Left in File Manager"), this);
    m_revealRightInFileManagerAction = new QAction(tr("Show Right in File Manager"), this);
    m_revealLeftParentInFileManagerAction = new QAction(tr("Show Left Parent in File Manager"), this);
    m_revealRightParentInFileManagerAction = new QAction(tr("Show Right Parent in File Manager"), this);
    m_mergeActionPlaceholder = new QAction(tr("Merge / Sync Action"), this);
    m_previewCopyLeftToRightAction = new QAction(tr("Preview Copy Left to Right"), this);
    m_previewCopyRightToLeftAction = new QAction(tr("Preview Copy Right to Left"), this);
    m_previewDeleteLeftAction = new QAction(tr("Preview Delete Left"), this);
    m_previewDeleteRightAction = new QAction(tr("Preview Delete Right"), this);
    m_previewCreateLeftDirectoryAction = new QAction(tr("Preview Create Left Directory"), this);
    m_previewCreateRightDirectoryAction = new QAction(tr("Preview Create Right Directory"), this);
    m_executeCopyLeftToRightAction = new QAction(tr("Apply Copy Left to Right"), this);
    m_executeCopyRightToLeftAction = new QAction(tr("Apply Copy Right to Left"), this);
    m_executeCreateLeftDirectoryAction = new QAction(tr("Apply Create Left Directory"), this);
    m_executeCreateRightDirectoryAction = new QAction(tr("Apply Create Right Directory"), this);
    m_exportReportAction = new QAction(tr("Export Report"), this);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(12, 12, 12, 12);
    rootLayout->setSpacing(10);

    auto *summaryLabel = new QLabel(
        tr("Compare two folders by relative path, file type, and file contents. This page is the folder-specific counterpart to the per-tab text compare workflow."),
        this);
    summaryLabel->setWordWrap(true);
    rootLayout->addWidget(summaryLabel);

    m_inputHeader = new CompareInputHeader(tr("Left folder"), tr("Right folder"), this);
    rootLayout->addWidget(m_inputHeader);

    auto *optionsRow = new QWidget(this);
    auto *optionsLayout = new QHBoxLayout(optionsRow);
    optionsLayout->setContentsMargins(0, 0, 0, 0);
    m_filterEdit = new QLineEdit(optionsRow);
    m_filterEdit->setPlaceholderText(tr("Filter by relative path"));
    m_filterModeCombo = new QComboBox(optionsRow);
    m_filterModeCombo->addItem(tr("All Entries"));
    m_filterModeCombo->addItem(tr("Differences Only"));
    m_filterModeCombo->addItem(tr("Identical Only"));
    m_filterModeCombo->addItem(tr("Left Only"));
    m_filterModeCombo->addItem(tr("Right Only"));
    m_filterModeCombo->addItem(tr("Different Content"));
    m_filterModeCombo->addItem(tr("Type Mismatch"));
    auto *showIdenticalButton = new QPushButton(tr("Show Identical Items"), optionsRow);
    showIdenticalButton->setCheckable(true);
    auto *openSelectedButton = new QPushButton(tr("Open Selected File Pair"), optionsRow);
    auto *openLeftOnlyButton = new QPushButton(tr("Open Left File Against Empty"), optionsRow);
    auto *openRightOnlyButton = new QPushButton(tr("Open Right File Against Empty"), optionsRow);
    auto *exportReportButton = new QPushButton(tr("Export Report"), optionsRow);
    optionsLayout->addWidget(m_filterEdit, 1);
    optionsLayout->addWidget(m_filterModeCombo);
    optionsLayout->addWidget(showIdenticalButton);
    optionsLayout->addWidget(openSelectedButton);
    optionsLayout->addWidget(openLeftOnlyButton);
    optionsLayout->addWidget(openRightOnlyButton);
    optionsLayout->addWidget(exportReportButton);
    optionsLayout->addStretch(1);
    rootLayout->addWidget(optionsRow);

    m_resultSummaryLabel = new QLabel(tr("Folder comparison results"), this);
    rootLayout->addWidget(m_resultSummaryLabel);

    m_operationPreviewLabel = new QLabel(tr("Select a result row to preview merge or sync operations."), this);
    m_operationPreviewLabel->setWordWrap(true);
    rootLayout->addWidget(m_operationPreviewLabel);

    m_lastOperationLabel = new QLabel(tr("No operation executed yet."), this);
    m_lastOperationLabel->setWordWrap(true);
    rootLayout->addWidget(m_lastOperationLabel);

    m_resultTable = new QTableWidget(this);
    m_resultTable->setColumnCount(6);
    m_resultTable->setHorizontalHeaderLabels(
        { tr("Status"), tr("Relative Path"), tr("Left Type"), tr("Right Type"), tr("Left Size"), tr("Right Size") });
    m_resultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resultTable->setContextMenuPolicy(Qt::CustomContextMenu);
    m_resultTable->verticalHeader()->setVisible(false);
    m_resultTable->horizontalHeader()->setStretchLastSection(true);
    m_resultTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_resultTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_resultTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_resultTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_resultTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_resultTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    rootLayout->addWidget(m_resultTable, 1);

    m_statusLabel = new QLabel(tr("Choose two folders before running folder compare."), this);
    m_statusLabel->setWordWrap(true);
    rootLayout->addWidget(m_statusLabel);

    connect(showIdenticalButton, &QPushButton::toggled, m_showIdenticalItemsAction, &QAction::setChecked);
    connect(m_showIdenticalItemsAction, &QAction::toggled, showIdenticalButton, &QPushButton::setChecked);
    connect(openSelectedButton, &QPushButton::clicked, m_openSelectedTextCompareAction, &QAction::trigger);
    connect(openLeftOnlyButton, &QPushButton::clicked, m_openSelectedLeftOnlyTextCompareAction, &QAction::trigger);
    connect(openRightOnlyButton, &QPushButton::clicked, m_openSelectedRightOnlyTextCompareAction, &QAction::trigger);
    connect(exportReportButton, &QPushButton::clicked, m_exportReportAction, &QAction::trigger);
    connect(m_inputHeader, &CompareInputHeader::leftBrowseRequested, this, &FolderCompareTabPage::chooseLeftFile);
    connect(m_inputHeader, &CompareInputHeader::rightBrowseRequested, this, &FolderCompareTabPage::chooseRightFile);
    connect(m_inputHeader, &CompareInputHeader::compareRequested, this, &FolderCompareTabPage::executeCompare);
    connect(m_inputHeader, &CompareInputHeader::reloadRequested, this, &FolderCompareTabPage::reloadInputs);
    connect(m_inputHeader, &CompareInputHeader::swapRequested, this, &FolderCompareTabPage::swapInputs);
}

void FolderCompareTabPage::connectSignals()
{
    auto *settings = mergeqt::app::SettingsService::instance();

    connect(m_showIdenticalItemsAction, &QAction::toggled, this, [this, settings](bool checked) {
        settings->compare()->set_showIdenticalItems(checked);
        refreshResultView();
    });
    connect(m_openSelectedTextCompareAction, &QAction::triggered, this, &FolderCompareTabPage::openSelectedEntryInTextCompare);
    connect(m_openSelectedLeftOnlyTextCompareAction,
            &QAction::triggered,
            this,
            &FolderCompareTabPage::openSelectedLeftOnlyInTextCompare);
    connect(m_openSelectedRightOnlyTextCompareAction,
            &QAction::triggered,
            this,
            &FolderCompareTabPage::openSelectedRightOnlyInTextCompare);
    connect(m_copyRelativePathAction, &QAction::triggered, this, &FolderCompareTabPage::copyRelativePath);
    connect(m_copyLeftFileNameAction, &QAction::triggered, this, &FolderCompareTabPage::copyLeftFileName);
    connect(m_copyRightFileNameAction, &QAction::triggered, this, &FolderCompareTabPage::copyRightFileName);
    connect(m_copyLeftAbsolutePathAction, &QAction::triggered, this, &FolderCompareTabPage::copyLeftAbsolutePath);
    connect(m_copyRightAbsolutePathAction, &QAction::triggered, this, &FolderCompareTabPage::copyRightAbsolutePath);
    connect(m_copyLeftParentPathAction, &QAction::triggered, this, &FolderCompareTabPage::copyLeftParentPath);
    connect(m_copyRightParentPathAction, &QAction::triggered, this, &FolderCompareTabPage::copyRightParentPath);
    connect(m_revealLeftInFileManagerAction, &QAction::triggered, this, &FolderCompareTabPage::revealLeftInFileManager);
    connect(m_revealRightInFileManagerAction, &QAction::triggered, this, &FolderCompareTabPage::revealRightInFileManager);
    connect(m_revealLeftParentInFileManagerAction,
            &QAction::triggered,
            this,
            &FolderCompareTabPage::revealLeftParentInFileManager);
    connect(m_revealRightParentInFileManagerAction,
            &QAction::triggered,
            this,
            &FolderCompareTabPage::revealRightParentInFileManager);
    connect(m_mergeActionPlaceholder, &QAction::triggered, this, &FolderCompareTabPage::showMergeActionPlaceholder);
    connect(m_previewCopyLeftToRightAction, &QAction::triggered, this, &FolderCompareTabPage::previewCopyLeftToRight);
    connect(m_previewCopyRightToLeftAction, &QAction::triggered, this, &FolderCompareTabPage::previewCopyRightToLeft);
    connect(m_previewDeleteLeftAction, &QAction::triggered, this, &FolderCompareTabPage::previewDeleteLeft);
    connect(m_previewDeleteRightAction, &QAction::triggered, this, &FolderCompareTabPage::previewDeleteRight);
    connect(m_previewCreateLeftDirectoryAction, &QAction::triggered, this, &FolderCompareTabPage::previewCreateLeftDirectory);
    connect(m_previewCreateRightDirectoryAction, &QAction::triggered, this, &FolderCompareTabPage::previewCreateRightDirectory);
    connect(m_executeCopyLeftToRightAction, &QAction::triggered, this, &FolderCompareTabPage::executeCopyLeftToRight);
    connect(m_executeCopyRightToLeftAction, &QAction::triggered, this, &FolderCompareTabPage::executeCopyRightToLeft);
    connect(m_executeCreateLeftDirectoryAction, &QAction::triggered, this, &FolderCompareTabPage::executeCreateLeftDirectory);
    connect(m_executeCreateRightDirectoryAction, &QAction::triggered, this, &FolderCompareTabPage::executeCreateRightDirectory);
    connect(m_exportReportAction, &QAction::triggered, this, &FolderCompareTabPage::exportReport);
    connect(m_filterEdit, &QLineEdit::textChanged, this, [this](const QString &) {
        refreshResultView();
    });
    connect(m_filterModeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) {
        refreshResultView();
    });

    connect(&m_compareSession, &mergeqt::app::FolderCompareSessionService::comparisonCompleted, this, &FolderCompareTabPage::refreshResultView);
    connect(&m_compareSession, &mergeqt::app::FolderCompareSessionService::comparisonFailed, this, [this](const QString &message) {
        m_statusLabel->setText(message);
        emit pageStatusChanged(message);
    });
    connect(&m_compareSession, &mergeqt::app::FolderCompareSessionService::comparisonStatusChanged, this, [this](const QString &message) {
        m_statusLabel->setText(message);
        emit pageStatusChanged(message);
    });
    connect(&m_compareSession,
            &mergeqt::app::FolderCompareSessionService::operationFinished,
            this,
            [this](const QString &, mergeqt::core::FolderMergeActionKind, bool success, const QString &message) {
                setOperationMessage(message, success);
            });
    connect(&m_compareSession,
            &mergeqt::app::FolderCompareSessionService::operationBatchFinished,
            this,
            [this](int totalCount, int successCount, int failureCount, const QString &message) {
                const QString summary =
                    tr("%1 | selected: %2, succeeded: %3, failed: %4").arg(message).arg(totalCount).arg(successCount).arg(failureCount);
                setOperationMessage(summary, failureCount == 0);
            });
    connect(m_resultTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        m_openSelectedTextCompareAction->setEnabled(canOpenSelectedEntryInTextCompare());
        m_openSelectedLeftOnlyTextCompareAction->setEnabled(canOpenSelectedLeftOnlyInTextCompare());
        m_openSelectedRightOnlyTextCompareAction->setEnabled(canOpenSelectedRightOnlyInTextCompare());
        m_previewCopyLeftToRightAction->setEnabled(canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::CopyLeftToRight));
        m_previewCopyRightToLeftAction->setEnabled(canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::CopyRightToLeft));
        m_previewDeleteLeftAction->setEnabled(canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::DeleteLeft));
        m_previewDeleteRightAction->setEnabled(canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::DeleteRight));
        m_previewCreateLeftDirectoryAction->setEnabled(
            canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::CreateLeftDirectory));
        m_previewCreateRightDirectoryAction->setEnabled(
            canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::CreateRightDirectory));
        m_executeCopyLeftToRightAction->setEnabled(canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::CopyLeftToRight));
        m_executeCopyRightToLeftAction->setEnabled(canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::CopyRightToLeft));
        m_executeCreateLeftDirectoryAction->setEnabled(
            canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::CreateLeftDirectory));
        m_executeCreateRightDirectoryAction->setEnabled(
            canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::CreateRightDirectory));
        m_copyRelativePathAction->setEnabled(!currentRelativePath().isEmpty());
        m_copyLeftFileNameAction->setEnabled(!currentLeftFileName().isEmpty());
        m_copyRightFileNameAction->setEnabled(!currentRightFileName().isEmpty());
        m_copyLeftAbsolutePathAction->setEnabled(!currentLeftAbsolutePath().isEmpty());
        m_copyRightAbsolutePathAction->setEnabled(!currentRightAbsolutePath().isEmpty());
        m_copyLeftParentPathAction->setEnabled(!currentLeftParentPath().isEmpty());
        m_copyRightParentPathAction->setEnabled(!currentRightParentPath().isEmpty());
        m_revealLeftInFileManagerAction->setEnabled(!currentLeftAbsolutePath().isEmpty());
        m_revealRightInFileManagerAction->setEnabled(!currentRightAbsolutePath().isEmpty());
        m_revealLeftParentInFileManagerAction->setEnabled(!currentLeftParentPath().isEmpty());
        m_revealRightParentInFileManagerAction->setEnabled(!currentRightParentPath().isEmpty());
        m_mergeActionPlaceholder->setEnabled(!currentRelativePath().isEmpty());
        emitPageState();
    });
    connect(m_resultTable, &QTableWidget::itemDoubleClicked, this, [this](QTableWidgetItem *) {
        if (canOpenSelectedEntryInTextCompare())
            openSelectedEntryInTextCompare();
        else if (canOpenSelectedLeftOnlyInTextCompare())
            openSelectedLeftOnlyInTextCompare();
        else if (canOpenSelectedRightOnlyInTextCompare())
            openSelectedRightOnlyInTextCompare();
    });
    connect(m_resultTable, &QTableWidget::customContextMenuRequested, this, &FolderCompareTabPage::showResultContextMenu);
}

void FolderCompareTabPage::loadStartupState()
{
    if (!m_descriptor.leftPath.isEmpty() || !m_descriptor.rightPath.isEmpty()) {
        m_inputHeader->leftPathEdit()->setText(m_descriptor.leftPath);
        m_inputHeader->rightPathEdit()->setText(m_descriptor.rightPath);
    }
    applyWindowSettings();

    if (!m_inputHeader->leftPathEdit()->text().isEmpty() && !m_inputHeader->rightPathEdit()->text().isEmpty())
        executeCompare();
}

void FolderCompareTabPage::refreshResultView()
{
    const auto result = m_compareSession.currentResult();
    const auto entries = filteredEntries();

    m_resultTable->setRowCount(0);
    int visibleDifferenceRow = -1;

    for (const auto &entry : entries) {
        const int row = m_resultTable->rowCount();
        m_resultTable->insertRow(row);

        auto *statusItem = new QTableWidgetItem(diffStatusText(entry.status));
        statusItem->setData(Qt::UserRole, entry.isDifference());
        statusItem->setData(Qt::UserRole + 1, entry.leftAbsolutePath);
        statusItem->setData(Qt::UserRole + 2, entry.rightAbsolutePath);
        statusItem->setData(Qt::UserRole + 3, entry.leftKind == mergeqt::core::FolderEntryKind::File);
        statusItem->setData(Qt::UserRole + 4, entry.rightKind == mergeqt::core::FolderEntryKind::File);
        m_resultTable->setItem(row, 0, statusItem);
        m_resultTable->setItem(row, 1, new QTableWidgetItem(entry.relativePath));
        m_resultTable->setItem(row, 2, new QTableWidgetItem(entryKindText(entry.leftKind)));
        m_resultTable->setItem(row, 3, new QTableWidgetItem(entryKindText(entry.rightKind)));
        m_resultTable->setItem(row, 4, new QTableWidgetItem(sizeText(entry.leftSize)));
        m_resultTable->setItem(row, 5, new QTableWidgetItem(sizeText(entry.rightSize)));

        if (visibleDifferenceRow < 0 && entry.isDifference())
            visibleDifferenceRow = row;
    }

    m_resultSummaryLabel->setText(
        tr("Entries: %1, visible: %2, differences: %3")
            .arg(result.entries.size())
            .arg(entries.size())
            .arg(result.stats.differenceCount()));

    if (visibleDifferenceRow >= 0)
        selectDifferenceRow(visibleDifferenceRow);
    else if (m_resultTable->rowCount() > 0)
        m_resultTable->setCurrentCell(0, 0);

    m_openSelectedTextCompareAction->setEnabled(canOpenSelectedEntryInTextCompare());
    m_openSelectedLeftOnlyTextCompareAction->setEnabled(canOpenSelectedLeftOnlyInTextCompare());
    m_openSelectedRightOnlyTextCompareAction->setEnabled(canOpenSelectedRightOnlyInTextCompare());
    m_previewCopyLeftToRightAction->setEnabled(canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::CopyLeftToRight));
    m_previewCopyRightToLeftAction->setEnabled(canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::CopyRightToLeft));
    m_previewDeleteLeftAction->setEnabled(canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::DeleteLeft));
    m_previewDeleteRightAction->setEnabled(canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::DeleteRight));
    m_previewCreateLeftDirectoryAction->setEnabled(canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::CreateLeftDirectory));
    m_previewCreateRightDirectoryAction->setEnabled(
        canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::CreateRightDirectory));
    m_executeCopyLeftToRightAction->setEnabled(canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::CopyLeftToRight));
    m_executeCopyRightToLeftAction->setEnabled(canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::CopyRightToLeft));
    m_executeCreateLeftDirectoryAction->setEnabled(
        canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::CreateLeftDirectory));
    m_executeCreateRightDirectoryAction->setEnabled(
        canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind::CreateRightDirectory));
    m_copyRelativePathAction->setEnabled(!currentRelativePath().isEmpty());
    m_copyLeftFileNameAction->setEnabled(!currentLeftFileName().isEmpty());
    m_copyRightFileNameAction->setEnabled(!currentRightFileName().isEmpty());
    m_copyLeftAbsolutePathAction->setEnabled(!currentLeftAbsolutePath().isEmpty());
    m_copyRightAbsolutePathAction->setEnabled(!currentRightAbsolutePath().isEmpty());
    m_copyLeftParentPathAction->setEnabled(!currentLeftParentPath().isEmpty());
    m_copyRightParentPathAction->setEnabled(!currentRightParentPath().isEmpty());
    m_revealLeftInFileManagerAction->setEnabled(!currentLeftAbsolutePath().isEmpty());
    m_revealRightInFileManagerAction->setEnabled(!currentRightAbsolutePath().isEmpty());
    m_revealLeftParentInFileManagerAction->setEnabled(!currentLeftParentPath().isEmpty());
    m_revealRightParentInFileManagerAction->setEnabled(!currentRightParentPath().isEmpty());
    m_mergeActionPlaceholder->setEnabled(!currentRelativePath().isEmpty());
    emitPageState();
}

void FolderCompareTabPage::selectDifferenceRow(int row)
{
    if (!m_resultTable || row < 0 || row >= m_resultTable->rowCount())
        return;

    m_resultTable->setCurrentCell(row, 0);
    m_resultTable->scrollToItem(m_resultTable->item(row, 0));
}

QTableWidgetItem *FolderCompareTabPage::currentStatusItem() const
{
    return m_resultTable ? m_resultTable->item(m_resultTable->currentRow(), 0) : nullptr;
}

QStringList FolderCompareTabPage::selectedRelativePaths() const
{
    QStringList paths;
    if (!m_resultTable || !m_resultTable->selectionModel())
        return paths;

    const auto rows = m_resultTable->selectionModel()->selectedRows(1);
    paths.reserve(rows.size());
    for (const auto &index : rows)
        paths.append(index.data().toString());
    return paths;
}

QString FolderCompareTabPage::currentRelativePath() const
{
    const auto *item = currentStatusItem();
    if (!item)
        return {};

    const auto *pathItem = m_resultTable->item(item->row(), 1);
    return pathItem ? pathItem->text() : QString{};
}

QString FolderCompareTabPage::currentLeftAbsolutePath() const
{
    const auto *item = currentStatusItem();
    return item ? item->data(Qt::UserRole + 1).toString() : QString{};
}

QString FolderCompareTabPage::currentRightAbsolutePath() const
{
    const auto *item = currentStatusItem();
    return item ? item->data(Qt::UserRole + 2).toString() : QString{};
}

QString FolderCompareTabPage::currentLeftFileName() const
{
    const QString path = currentLeftAbsolutePath();
    return path.isEmpty() ? QString{} : QFileInfo(path).fileName();
}

QString FolderCompareTabPage::currentRightFileName() const
{
    const QString path = currentRightAbsolutePath();
    return path.isEmpty() ? QString{} : QFileInfo(path).fileName();
}

QString FolderCompareTabPage::currentLeftParentPath() const
{
    const QString path = currentLeftAbsolutePath();
    return path.isEmpty() ? QString{} : QFileInfo(path).absolutePath();
}

QString FolderCompareTabPage::currentRightParentPath() const
{
    const QString path = currentRightAbsolutePath();
    return path.isEmpty() ? QString{} : QFileInfo(path).absolutePath();
}

mergeqt::core::FolderOperationPlan FolderCompareTabPage::previewCurrentOperation(
    mergeqt::core::FolderMergeActionKind actionKind) const
{
    if (currentRelativePath().isEmpty())
        return {};

    return m_compareSession.previewOperation(currentRelativePath(), actionKind);
}

mergeqt::core::FolderOperationBatch FolderCompareTabPage::previewSelectedOperations(
    mergeqt::core::FolderMergeActionKind actionKind) const
{
    return m_compareSession.previewOperations(selectedRelativePaths(), actionKind);
}

bool FolderCompareTabPage::currentLeftIsFile() const
{
    const auto *item = currentStatusItem();
    return item ? item->data(Qt::UserRole + 3).toBool() : false;
}

bool FolderCompareTabPage::currentRightIsFile() const
{
    const auto *item = currentStatusItem();
    return item ? item->data(Qt::UserRole + 4).toBool() : false;
}

bool FolderCompareTabPage::hasSelectedRows() const
{
    return !selectedRelativePaths().isEmpty();
}

bool FolderCompareTabPage::canOpenSelectedEntryInTextCompare() const
{
    return currentLeftIsFile() && currentRightIsFile();
}

bool FolderCompareTabPage::canOpenSelectedLeftOnlyInTextCompare() const
{
    return currentLeftIsFile() && !currentRightIsFile();
}

bool FolderCompareTabPage::canOpenSelectedRightOnlyInTextCompare() const
{
    return !currentLeftIsFile() && currentRightIsFile();
}

bool FolderCompareTabPage::canPreviewSelectedOperations(mergeqt::core::FolderMergeActionKind actionKind) const
{
    return previewSelectedOperations(actionKind).isValid();
}

void FolderCompareTabPage::openSelectedEntryInTextCompare()
{
    if (!canOpenSelectedEntryInTextCompare()) {
        const QString message = tr("Select a row where both sides are files to open a text compare tab.");
        m_statusLabel->setText(message);
        emit pageStatusChanged(message);
        return;
    }

    mergeqt::app::ComparePageDescriptor descriptor;
    descriptor.type = mergeqt::app::ComparePageType::Text;
    descriptor.leftPath = currentLeftAbsolutePath();
    descriptor.rightPath = currentRightAbsolutePath();
    descriptor.title = tr("Text Compare");
    setStatusMessage(tr("Opened selected file pair in a new text compare tab."));
    emit openCompareRequested(descriptor);
}

void FolderCompareTabPage::openSelectedLeftOnlyInTextCompare()
{
    if (!canOpenSelectedLeftOnlyInTextCompare()) {
        const QString message = tr("Select a row with a left-side file to compare it against empty content.");
        setStatusMessage(message);
        return;
    }

    mergeqt::app::ComparePageDescriptor descriptor;
    descriptor.type = mergeqt::app::ComparePageType::Text;
    descriptor.leftPath = currentLeftAbsolutePath();
    descriptor.title = tr("Text Compare");
    setStatusMessage(tr("Opened the selected left-side file against empty content in a new text compare tab."));
    emit openCompareRequested(descriptor);
}

void FolderCompareTabPage::openSelectedRightOnlyInTextCompare()
{
    if (!canOpenSelectedRightOnlyInTextCompare()) {
        const QString message = tr("Select a row with a right-side file to compare it against empty content.");
        setStatusMessage(message);
        return;
    }

    mergeqt::app::ComparePageDescriptor descriptor;
    descriptor.type = mergeqt::app::ComparePageType::Text;
    descriptor.rightPath = currentRightAbsolutePath();
    descriptor.title = tr("Text Compare");
    setStatusMessage(tr("Opened the selected right-side file against empty content in a new text compare tab."));
    emit openCompareRequested(descriptor);
}

void FolderCompareTabPage::copyRelativePath()
{
    copyTextToClipboard(currentRelativePath(),
                        tr("Copied relative path to the clipboard."),
                        tr("Select a comparison row before copying the relative path."));
}

void FolderCompareTabPage::copyLeftFileName()
{
    copyTextToClipboard(currentLeftFileName(),
                        tr("Copied the left-side file name to the clipboard."),
                        tr("Select a row with a left-side file path before copying its file name."));
}

void FolderCompareTabPage::copyRightFileName()
{
    copyTextToClipboard(currentRightFileName(),
                        tr("Copied the right-side file name to the clipboard."),
                        tr("Select a row with a right-side file path before copying its file name."));
}

void FolderCompareTabPage::copyLeftAbsolutePath()
{
    copyTextToClipboard(currentLeftAbsolutePath(),
                        tr("Copied the left-side path to the clipboard."),
                        tr("Select a row with a left-side path before copying it."));
}

void FolderCompareTabPage::copyRightAbsolutePath()
{
    copyTextToClipboard(currentRightAbsolutePath(),
                        tr("Copied the right-side path to the clipboard."),
                        tr("Select a row with a right-side path before copying it."));
}

void FolderCompareTabPage::copyLeftParentPath()
{
    copyTextToClipboard(currentLeftParentPath(),
                        tr("Copied the left-side parent path to the clipboard."),
                        tr("Select a row with a left-side path before copying its parent path."));
}

void FolderCompareTabPage::copyRightParentPath()
{
    copyTextToClipboard(currentRightParentPath(),
                        tr("Copied the right-side parent path to the clipboard."),
                        tr("Select a row with a right-side path before copying its parent path."));
}

void FolderCompareTabPage::revealLeftInFileManager()
{
    openPathInFileManager(currentLeftAbsolutePath(),
                          tr("Opened the left-side path in the system file manager."),
                          tr("Select a row with a left-side path before opening it in the file manager."));
}

void FolderCompareTabPage::revealRightInFileManager()
{
    openPathInFileManager(currentRightAbsolutePath(),
                          tr("Opened the right-side path in the system file manager."),
                          tr("Select a row with a right-side path before opening it in the file manager."));
}

void FolderCompareTabPage::revealLeftParentInFileManager()
{
    openPathInFileManager(currentLeftParentPath(),
                          tr("Opened the left-side parent path in the system file manager."),
                          tr("Select a row with a left-side path before opening its parent path in the file manager."));
}

void FolderCompareTabPage::revealRightParentInFileManager()
{
    openPathInFileManager(currentRightParentPath(),
                          tr("Opened the right-side parent path in the system file manager."),
                          tr("Select a row with a right-side path before opening its parent path in the file manager."));
}

void FolderCompareTabPage::showMergeActionPlaceholder()
{
    if (currentRelativePath().isEmpty()) {
        setStatusMessage(tr("Select a comparison row before opening a merge or sync action."));
        return;
    }

    setStatusMessage(tr("Merge and sync commands are reserved for the next implementation phase."));
}

void FolderCompareTabPage::previewCopyLeftToRight()
{
    showOperationBatchPreview(previewSelectedOperations(mergeqt::core::FolderMergeActionKind::CopyLeftToRight),
                              tr("Select one or more rows that can preview a left-to-right copy plan."));
}

void FolderCompareTabPage::previewCopyRightToLeft()
{
    showOperationBatchPreview(previewSelectedOperations(mergeqt::core::FolderMergeActionKind::CopyRightToLeft),
                              tr("Select one or more rows that can preview a right-to-left copy plan."));
}

void FolderCompareTabPage::previewDeleteLeft()
{
    showOperationBatchPreview(previewSelectedOperations(mergeqt::core::FolderMergeActionKind::DeleteLeft),
                              tr("Select one or more rows that can preview deleting the left-side item."));
}

void FolderCompareTabPage::previewDeleteRight()
{
    showOperationBatchPreview(previewSelectedOperations(mergeqt::core::FolderMergeActionKind::DeleteRight),
                              tr("Select one or more rows that can preview deleting the right-side item."));
}

void FolderCompareTabPage::previewCreateLeftDirectory()
{
    showOperationBatchPreview(previewSelectedOperations(mergeqt::core::FolderMergeActionKind::CreateLeftDirectory),
                              tr("Select one or more rows that can preview creating a missing left-side directory."));
}

void FolderCompareTabPage::previewCreateRightDirectory()
{
    showOperationBatchPreview(previewSelectedOperations(mergeqt::core::FolderMergeActionKind::CreateRightDirectory),
                              tr("Select one or more rows that can preview creating a missing right-side directory."));
}

void FolderCompareTabPage::executeCopyLeftToRight()
{
    confirmAndExecuteBatch(previewSelectedOperations(mergeqt::core::FolderMergeActionKind::CopyLeftToRight),
                           tr("Apply Copy Left to Right"));
}

void FolderCompareTabPage::executeCopyRightToLeft()
{
    confirmAndExecuteBatch(previewSelectedOperations(mergeqt::core::FolderMergeActionKind::CopyRightToLeft),
                           tr("Apply Copy Right to Left"));
}

void FolderCompareTabPage::executeCreateLeftDirectory()
{
    confirmAndExecuteBatch(previewSelectedOperations(mergeqt::core::FolderMergeActionKind::CreateLeftDirectory),
                           tr("Apply Create Left Directory"));
}

void FolderCompareTabPage::executeCreateRightDirectory()
{
    confirmAndExecuteBatch(previewSelectedOperations(mergeqt::core::FolderMergeActionKind::CreateRightDirectory),
                           tr("Apply Create Right Directory"));
}

void FolderCompareTabPage::showResultContextMenu(const QPoint &position)
{
    if (!m_resultTable)
        return;

    if (auto *item = m_resultTable->itemAt(position))
        m_resultTable->setCurrentItem(item);

    QMenu menu(this);
    menu.addAction(m_openSelectedTextCompareAction);
    menu.addAction(m_openSelectedLeftOnlyTextCompareAction);
    menu.addAction(m_openSelectedRightOnlyTextCompareAction);
    menu.addSeparator();
    menu.addAction(m_previewCopyLeftToRightAction);
    menu.addAction(m_previewCopyRightToLeftAction);
    menu.addAction(m_previewDeleteLeftAction);
    menu.addAction(m_previewDeleteRightAction);
    menu.addAction(m_previewCreateLeftDirectoryAction);
    menu.addAction(m_previewCreateRightDirectoryAction);
    menu.addSeparator();
    menu.addAction(m_executeCopyLeftToRightAction);
    menu.addAction(m_executeCopyRightToLeftAction);
    menu.addAction(m_executeCreateLeftDirectoryAction);
    menu.addAction(m_executeCreateRightDirectoryAction);
    menu.addSeparator();
    menu.addAction(m_exportReportAction);
    menu.addSeparator();
    menu.addAction(m_copyRelativePathAction);
    menu.addAction(m_copyLeftFileNameAction);
    menu.addAction(m_copyRightFileNameAction);
    menu.addAction(m_copyLeftAbsolutePathAction);
    menu.addAction(m_copyRightAbsolutePathAction);
    menu.addAction(m_copyLeftParentPathAction);
    menu.addAction(m_copyRightParentPathAction);
    menu.addSeparator();
    menu.addAction(m_revealLeftInFileManagerAction);
    menu.addAction(m_revealRightInFileManagerAction);
    menu.addAction(m_revealLeftParentInFileManagerAction);
    menu.addAction(m_revealRightParentInFileManagerAction);
    menu.addSeparator();
    menu.addAction(m_mergeActionPlaceholder);
    menu.exec(m_resultTable->viewport()->mapToGlobal(position));
}

QString FolderCompareTabPage::entryKindText(mergeqt::core::FolderEntryKind kind) const
{
    switch (kind) {
    case mergeqt::core::FolderEntryKind::Missing:
        return tr("Missing");
    case mergeqt::core::FolderEntryKind::File:
        return tr("File");
    case mergeqt::core::FolderEntryKind::Directory:
        return tr("Folder");
    case mergeqt::core::FolderEntryKind::Other:
        return tr("Other");
    }

    return tr("Other");
}

QString FolderCompareTabPage::diffStatusText(mergeqt::core::FolderDiffStatus status) const
{
    switch (status) {
    case mergeqt::core::FolderDiffStatus::Identical:
        return tr("Identical");
    case mergeqt::core::FolderDiffStatus::LeftOnly:
        return tr("Left Only");
    case mergeqt::core::FolderDiffStatus::RightOnly:
        return tr("Right Only");
    case mergeqt::core::FolderDiffStatus::ContentMismatch:
        return tr("Different Content");
    case mergeqt::core::FolderDiffStatus::TypeMismatch:
        return tr("Type Mismatch");
    }

    return tr("Type Mismatch");
}

QString FolderCompareTabPage::sizeText(qint64 size) const
{
    return size < 0 ? QStringLiteral("-") : QString::number(size);
}

QString FolderCompareTabPage::previewStepKindText(mergeqt::core::FolderOperationStepKind stepKind) const
{
    switch (stepKind) {
    case mergeqt::core::FolderOperationStepKind::EnsureParentDirectory:
        return tr("Ensure Parent");
    case mergeqt::core::FolderOperationStepKind::CreateDirectory:
        return tr("Create Directory");
    case mergeqt::core::FolderOperationStepKind::CopyFile:
        return tr("Copy File");
    case mergeqt::core::FolderOperationStepKind::DeleteFile:
        return tr("Delete File");
    case mergeqt::core::FolderOperationStepKind::RemoveDirectory:
        return tr("Remove Directory");
    }

    return tr("Copy File");
}

void FolderCompareTabPage::setStatusMessage(const QString &message)
{
    m_statusLabel->setText(message);
    emit pageStatusChanged(message);
}

void FolderCompareTabPage::exportReport()
{
    const auto entries = filteredEntries();
    if (entries.isEmpty()) {
        setStatusMessage(tr("There are no visible folder compare entries to export."));
        return;
    }

    const QString filePath = QFileDialog::getSaveFileName(this,
                                                          tr("Export folder compare report"),
                                                          tr("folder-compare-report"),
                                                          tr("HTML Report (*.html);;Text Report (*.txt)"));
    if (filePath.isEmpty())
        return;

    const auto format = filePath.endsWith(QStringLiteral(".html"), Qt::CaseInsensitive)
                            ? mergeqt::app::FolderCompareReportFormat::Html
                            : mergeqt::app::FolderCompareReportFormat::Text;
    const QString content =
        mergeqt::app::FolderCompareReportFormatter::formatReport(m_compareSession.currentResult(), entries, format);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        setStatusMessage(tr("Failed to write report to %1.").arg(filePath));
        return;
    }

    file.write(content.toUtf8());
    setStatusMessage(tr("Exported folder compare report to %1.").arg(filePath));
}

QVector<mergeqt::core::FolderCompareEntry> FolderCompareTabPage::filteredEntries() const
{
    QVector<mergeqt::core::FolderCompareEntry> entries;
    const auto result = m_compareSession.currentResult();
    entries.reserve(result.entries.size());
    for (const auto &entry : result.entries) {
        if (entryMatchesFilter(entry))
            entries.append(entry);
    }
    return entries;
}

bool FolderCompareTabPage::entryMatchesFilter(const mergeqt::core::FolderCompareEntry &entry) const
{
    const bool showIdentical = mergeqt::app::SettingsService::instance()->compare()->showIdenticalItems();
    if (!showIdentical && !entry.isDifference())
        return false;

    const QString filterText = m_filterEdit ? m_filterEdit->text().trimmed() : QString{};
    if (!filterText.isEmpty() && !entry.relativePath.contains(filterText, Qt::CaseInsensitive))
        return false;

    const int mode = m_filterModeCombo ? m_filterModeCombo->currentIndex() : 0;
    switch (mode) {
    case 0:
        return true;
    case 1:
        return entry.isDifference();
    case 2:
        return !entry.isDifference();
    case 3:
        return entry.status == mergeqt::core::FolderDiffStatus::LeftOnly;
    case 4:
        return entry.status == mergeqt::core::FolderDiffStatus::RightOnly;
    case 5:
        return entry.status == mergeqt::core::FolderDiffStatus::ContentMismatch;
    case 6:
        return entry.status == mergeqt::core::FolderDiffStatus::TypeMismatch;
    default:
        return true;
    }
}

bool FolderCompareTabPage::copyTextToClipboard(const QString &text,
                                               const QString &successMessage,
                                               const QString &failureMessage)
{
    if (text.isEmpty()) {
        setStatusMessage(failureMessage);
        return false;
    }

    if (auto *clipboard = QGuiApplication::clipboard())
        clipboard->setText(text);
    setStatusMessage(successMessage);
    return true;
}

bool FolderCompareTabPage::openPathInFileManager(const QString &path,
                                                 const QString &successMessage,
                                                 const QString &failureMessage)
{
    if (path.isEmpty()) {
        setStatusMessage(failureMessage);
        return false;
    }

    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(path))) {
        setStatusMessage(failureMessage);
        return false;
    }

    setStatusMessage(successMessage);
    return true;
}

void FolderCompareTabPage::showOperationPreview(const mergeqt::core::FolderOperationPlan &plan, const QString &emptyMessage)
{
    if (!plan.isValid || plan.steps.isEmpty()) {
        m_operationPreviewLabel->setText(emptyMessage);
        setStatusMessage(emptyMessage);
        return;
    }

    QStringList lines;
    lines.append(tr("%1: %2").arg(plan.summary, plan.relativePath));
    for (const auto &step : plan.steps) {
        QString detail = previewStepKindText(step.stepKind);
        if (!step.sourcePath.isEmpty() && !step.targetPath.isEmpty())
            detail += tr(" | %1 -> %2").arg(step.sourcePath, step.targetPath);
        else if (!step.sourcePath.isEmpty())
            detail += tr(" | %1").arg(step.sourcePath);
        else if (!step.targetPath.isEmpty())
            detail += tr(" | %1").arg(step.targetPath);
        if (!step.description.isEmpty())
            detail += tr(" | %1").arg(step.description);
        lines.append(detail);
    }

    const QString previewText = lines.join(QLatin1Char('\n'));
    m_operationPreviewLabel->setText(previewText);
    setStatusMessage(tr("Previewed operation plan for %1.").arg(plan.relativePath));
}

void FolderCompareTabPage::showOperationBatchPreview(const mergeqt::core::FolderOperationBatch &batch, const QString &emptyMessage)
{
    if (!batch.isValid()) {
        m_operationPreviewLabel->setText(emptyMessage);
        setStatusMessage(emptyMessage);
        return;
    }

    if (batch.plans.size() == 1) {
        showOperationPreview(batch.plans.constFirst(), emptyMessage);
        return;
    }

    QStringList lines;
    lines.append(tr("Planned operations: %1").arg(batch.plans.size()));
    for (const auto &plan : batch.plans) {
        lines.append(tr("%1: %2").arg(plan.summary, plan.relativePath));
        for (const auto &step : plan.steps) {
            QString detail = previewStepKindText(step.stepKind);
            if (!step.sourcePath.isEmpty() && !step.targetPath.isEmpty())
                detail += tr(" | %1 -> %2").arg(step.sourcePath, step.targetPath);
            else if (!step.sourcePath.isEmpty())
                detail += tr(" | %1").arg(step.sourcePath);
            else if (!step.targetPath.isEmpty())
                detail += tr(" | %1").arg(step.targetPath);
            lines.append(detail);
        }
    }

    m_operationPreviewLabel->setText(lines.join(QLatin1Char('\n')));
    setStatusMessage(tr("Previewed %1 operation plans.").arg(batch.plans.size()));
}

bool FolderCompareTabPage::confirmAndExecuteOperation(const mergeqt::core::FolderOperationPlan &plan, const QString &title)
{
    if (!plan.isValid || plan.steps.isEmpty()) {
        setStatusMessage(tr("Select a row with a valid operation plan before executing it."));
        return false;
    }

    showOperationPreview(plan, {});
    const auto reply = QMessageBox::question(this,
                                             title,
                                             tr("Execute the following operation?\n\n%1").arg(m_operationPreviewLabel->text()),
                                             QMessageBox::Yes | QMessageBox::No,
                                             QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        setStatusMessage(tr("Canceled operation for %1.").arg(plan.relativePath));
        return false;
    }

    if (!m_compareSession.executeOperation(plan.relativePath, plan.actionKind))
        return false;

    setStatusMessage(tr("Executed operation for %1.").arg(plan.relativePath));
    return true;
}

bool FolderCompareTabPage::confirmAndExecuteBatch(const mergeqt::core::FolderOperationBatch &batch, const QString &title)
{
    if (!batch.isValid()) {
        setStatusMessage(tr("Select one or more rows with valid operation plans before executing them."));
        return false;
    }

    if (batch.plans.size() == 1)
        return confirmAndExecuteOperation(batch.plans.constFirst(), title);

    showOperationBatchPreview(batch, {});
    const auto reply = QMessageBox::question(this,
                                             title,
                                             tr("Execute the following %1 operations?\n\n%2")
                                                 .arg(batch.plans.size())
                                                 .arg(m_operationPreviewLabel->text()),
                                             QMessageBox::Yes | QMessageBox::No,
                                             QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        setStatusMessage(tr("Canceled %1 planned operations.").arg(batch.plans.size()));
        return false;
    }

    if (!m_compareSession.executeOperations(batch.relativePaths(), batch.plans.constFirst().actionKind))
        return false;

    setStatusMessage(tr("Executed %1 planned operations.").arg(batch.plans.size()));
    return true;
}

void FolderCompareTabPage::setOperationMessage(const QString &message, bool success)
{
    if (!m_lastOperationLabel)
        return;

    const QString prefix = success ? tr("Last operation: %1") : tr("Last operation failed: %1");
    m_lastOperationLabel->setText(prefix.arg(message));
}

void FolderCompareTabPage::emitPageState()
{
    emit pageTitleChanged(pageTitle());
    emit pageStatusChanged(pageStatusText());
}

} // namespace mergeqt::ui

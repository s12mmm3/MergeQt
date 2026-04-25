#include "ui/widgets/textcomparetabpage.h"

#include "app/settings/settingsservice.h"
#include "ui/widgets/compareinputheader.h"
#include "ui/widgets/textdiffoverviewbar.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTextBlock>
#include <QTextEdit>
#include <QVBoxLayout>

namespace mergeqt::ui {

TextCompareTabPage::TextCompareTabPage(const mergeqt::app::ComparePageDescriptor &descriptor, QWidget *parent)
    : CompareTabPage(parent)
    , m_compareSession(this)
    , m_descriptor(descriptor)
{
    buildUi();
    connectSignals();
    loadStartupState();
    executeCompare();
}

mergeqt::app::ComparePageType TextCompareTabPage::pageType() const
{
    return mergeqt::app::ComparePageType::Text;
}

QString TextCompareTabPage::pageTitle() const
{
    const auto leftDocument = m_compareSession.currentLeftDocument();
    const auto rightDocument = m_compareSession.currentRightDocument();
    const QString left = leftDocument.label.isEmpty() ? tr("Left") : leftDocument.label;
    const QString right = rightDocument.label.isEmpty() ? tr("Right") : rightDocument.label;
    return tr("%1 vs %2").arg(left, right);
}

QString TextCompareTabPage::pageStatusText() const
{
    return m_statusLabel ? m_statusLabel->text() : QString();
}

CompareTabCapabilities TextCompareTabPage::capabilities() const
{
    return { true, true, true, true, true };
}

bool TextCompareTabPage::chooseLeftFile()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Open left file"), m_inputHeader->leftPathEdit()->text());
    if (path.isEmpty())
        return false;

    m_inputHeader->leftPathEdit()->setText(path);
    return executeCompare();
}

bool TextCompareTabPage::chooseRightFile()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Open right file"), m_inputHeader->rightPathEdit()->text());
    if (path.isEmpty())
        return false;

    m_inputHeader->rightPathEdit()->setText(path);
    return executeCompare();
}

bool TextCompareTabPage::reloadInputs()
{
    if (!m_compareSession.reloadCurrentFiles())
        return false;

    const auto input = m_compareSession.currentInput();
    m_inputHeader->leftPathEdit()->setText(m_compareSession.currentLeftDocument().sourcePath);
    m_inputHeader->rightPathEdit()->setText(m_compareSession.currentRightDocument().sourcePath);
    refreshDocumentMeta();
    updateAlignedEditors();
    emitPageState();
    return true;
}

bool TextCompareTabPage::swapInputs()
{
    if (!m_compareSession.swapInputs())
        return false;

    const auto input = m_compareSession.currentInput();
    const auto leftDocument = m_compareSession.currentLeftDocument();
    const auto rightDocument = m_compareSession.currentRightDocument();
    m_inputHeader->leftPathEdit()->setText(leftDocument.sourcePath);
    m_inputHeader->rightPathEdit()->setText(rightDocument.sourcePath);
    refreshDocumentMeta();
    updateAlignedEditors();
    emitPageState();
    return true;
}

bool TextCompareTabPage::executeCompare()
{
    const QString leftPath = m_inputHeader->leftPathEdit()->text().trimmed();
    const QString rightPath = m_inputHeader->rightPathEdit()->text().trimmed();

    const bool ok = executeCompareFromDescriptorOrPaths(leftPath, rightPath);

    if (!ok)
        return false;

    refreshDocumentMeta();
    emitPageState();
    return true;
}

void TextCompareTabPage::navigateDifferenceByOffset(int offset)
{
    if (m_diffList->count() <= 0)
        return;

    int row = m_diffList->currentRow();
    if (row < 0)
        row = 0;
    else
        row = (row + offset + m_diffList->count()) % m_diffList->count();

    selectDifferenceRow(row);
}

void TextCompareTabPage::applyWindowSettings()
{
    updateCompareOptionsFromSettings();
}

void TextCompareTabPage::retranslateUi()
{
    if (m_pageSummaryLabel) {
        m_pageSummaryLabel->setText(
            tr("Edit either side directly or load two files, then run the minimal line diff from the per-tab compare session."));
    }
    if (m_inputHeader)
        m_inputHeader->setLabelTexts(tr("Left source"), tr("Right source"));
    if (m_inputHeader)
        m_inputHeader->retranslateUi();
    if (m_leftEditor)
        m_leftEditor->setPlaceholderText(tr("Left text"));
    if (m_rightEditor)
        m_rightEditor->setPlaceholderText(tr("Right text"));
    if (m_ignoreCaseAction)
        m_ignoreCaseAction->setText(tr("Ignore Case"));
    if (m_trimWhitespaceAction)
        m_trimWhitespaceAction->setText(tr("Trim Whitespace"));
    if (m_showIdenticalItemsAction)
        m_showIdenticalItemsAction->setText(tr("Show Identical Rows"));
    if (m_ignoreCaseButton)
        m_ignoreCaseButton->setText(tr("Ignore Case"));
    if (m_trimWhitespaceButton)
        m_trimWhitespaceButton->setText(tr("Trim Whitespace"));
    if (m_showIdenticalButton)
        m_showIdenticalButton->setText(tr("Show Identical Rows"));
    if (!m_compareSession.currentResult().lines.isEmpty()) {
        refreshDocumentMeta();
        refreshResultView();
    }
}

void TextCompareTabPage::buildUi()
{
    m_ignoreCaseAction = new QAction(this);
    m_ignoreCaseAction->setCheckable(true);
    m_trimWhitespaceAction = new QAction(this);
    m_trimWhitespaceAction->setCheckable(true);
    m_showIdenticalItemsAction = new QAction(this);
    m_showIdenticalItemsAction->setCheckable(true);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(12, 12, 12, 12);
    rootLayout->setSpacing(10);

    m_pageSummaryLabel = new QLabel(this);
    m_pageSummaryLabel->setWordWrap(true);
    rootLayout->addWidget(m_pageSummaryLabel);

    m_inputHeader = new CompareInputHeader(QString(), QString(), this);
    rootLayout->addWidget(m_inputHeader);

    auto *editorPanel = new QWidget(this);
    auto *editorLayout = new QVBoxLayout(editorPanel);
    editorLayout->setContentsMargins(0, 0, 0, 0);
    editorLayout->setSpacing(8);
    m_editorSplitter = new QSplitter(Qt::Horizontal, editorPanel);
    m_editorSplitter->setChildrenCollapsible(false);
    m_leftEditor = new LineNumberTextEdit(m_editorSplitter);
    m_leftEditor->setObjectName(QStringLiteral("leftCompareEditor"));
    m_leftEditor->setPlaceholderText(QString());
    m_diffOverviewBar = new TextDiffOverviewBar(m_editorSplitter);
    m_diffOverviewBar->setMinimumWidth(18);
    m_diffOverviewBar->setMaximumWidth(64);
    m_rightEditor = new LineNumberTextEdit(m_editorSplitter);
    m_rightEditor->setObjectName(QStringLiteral("rightCompareEditor"));
    m_rightEditor->setPlaceholderText(QString());
    m_leftEditor->addSynchronizedEditor(m_rightEditor);
    m_rightEditor->addSynchronizedEditor(m_leftEditor);
    m_editorSplitter->setStretchFactor(0, 1);
    m_editorSplitter->setStretchFactor(1, 0);
    m_editorSplitter->setStretchFactor(2, 1);
    editorLayout->addWidget(m_editorSplitter, 1);

    auto *metaRow = new QWidget(this);
    auto *metaLayout = new QHBoxLayout(metaRow);
    metaLayout->setContentsMargins(0, 0, 0, 0);
    m_leftMetaLabel = new QLabel(metaRow);
    m_rightMetaLabel = new QLabel(metaRow);
    m_leftMetaLabel->setWordWrap(true);
    m_rightMetaLabel->setWordWrap(true);
    metaLayout->addWidget(m_leftMetaLabel, 1);
    metaLayout->addWidget(m_rightMetaLabel, 1);
    editorLayout->addWidget(metaRow);

    m_contentSplitter = new QSplitter(Qt::Vertical, this);
    m_contentSplitter->setChildrenCollapsible(false);
    m_contentSplitter->addWidget(editorPanel);

    auto *diffPanel = new QWidget(m_contentSplitter);
    auto *diffLayout = new QVBoxLayout(diffPanel);
    diffLayout->setContentsMargins(0, 0, 0, 0);
    diffLayout->setSpacing(8);
    m_resultSummaryLabel = new QLabel(diffPanel);
    diffLayout->addWidget(m_resultSummaryLabel);

    m_diffList = new QListWidget(diffPanel);
    m_diffList->setMinimumHeight(180);
    diffLayout->addWidget(m_diffList, 1);
    m_contentSplitter->setStretchFactor(0, 4);
    m_contentSplitter->setStretchFactor(1, 1);
    rootLayout->addWidget(m_contentSplitter, 1);

    auto *optionsRow = new QWidget(this);
    auto *optionsLayout = new QHBoxLayout(optionsRow);
    optionsLayout->setContentsMargins(0, 0, 0, 0);
    m_ignoreCaseButton = new QPushButton(optionsRow);
    m_ignoreCaseButton->setCheckable(true);
    m_trimWhitespaceButton = new QPushButton(optionsRow);
    m_trimWhitespaceButton->setCheckable(true);
    m_showIdenticalButton = new QPushButton(optionsRow);
    m_showIdenticalButton->setCheckable(true);
    optionsLayout->addWidget(m_ignoreCaseButton);
    optionsLayout->addWidget(m_trimWhitespaceButton);
    optionsLayout->addWidget(m_showIdenticalButton);
    optionsLayout->addStretch(1);
    rootLayout->addWidget(optionsRow);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    rootLayout->addWidget(m_statusLabel);

    connect(m_ignoreCaseButton, &QPushButton::toggled, m_ignoreCaseAction, &QAction::setChecked);
    connect(m_trimWhitespaceButton, &QPushButton::toggled, m_trimWhitespaceAction, &QAction::setChecked);
    connect(m_showIdenticalButton, &QPushButton::toggled, m_showIdenticalItemsAction, &QAction::setChecked);
    connect(m_ignoreCaseAction, &QAction::toggled, m_ignoreCaseButton, &QPushButton::setChecked);
    connect(m_trimWhitespaceAction, &QAction::toggled, m_trimWhitespaceButton, &QPushButton::setChecked);
    connect(m_showIdenticalItemsAction, &QAction::toggled, m_showIdenticalButton, &QPushButton::setChecked);
    connect(m_inputHeader, &CompareInputHeader::leftBrowseRequested, this, &TextCompareTabPage::chooseLeftFile);
    connect(m_inputHeader, &CompareInputHeader::rightBrowseRequested, this, &TextCompareTabPage::chooseRightFile);
    connect(m_inputHeader, &CompareInputHeader::compareRequested, this, &TextCompareTabPage::executeCompare);
    connect(m_inputHeader, &CompareInputHeader::reloadRequested, this, &TextCompareTabPage::reloadInputs);
    connect(m_inputHeader, &CompareInputHeader::swapRequested, this, &TextCompareTabPage::swapInputs);
    retranslateUi();
}

void TextCompareTabPage::connectSignals()
{
    auto *settings = mergeqt::app::SettingsService::instance();

    connect(m_ignoreCaseAction, &QAction::toggled, this, [this, settings](bool checked) {
        settings->compare()->set_ignoreCase(checked);
        executeCompare();
    });
    connect(m_trimWhitespaceAction, &QAction::toggled, this, [this, settings](bool checked) {
        settings->compare()->set_trimWhitespace(checked);
        executeCompare();
    });
    connect(m_showIdenticalItemsAction, &QAction::toggled, this, [this, settings](bool checked) {
        settings->compare()->set_showIdenticalItems(checked);
        refreshResultView();
    });

    connect(&m_compareSession, &mergeqt::app::CompareSessionService::comparisonCompleted, this, &TextCompareTabPage::refreshResultView);
    connect(&m_compareSession, &mergeqt::app::CompareSessionService::comparisonFailed, this, [this](const QString &message) {
        m_statusLabel->setText(message);
        emit pageStatusChanged(message);
    });
    connect(&m_compareSession, &mergeqt::app::CompareSessionService::comparisonStatusChanged, this, [this](const QString &message) {
        m_statusLabel->setText(message);
        emit pageStatusChanged(message);
    });
    connect(m_diffList, &QListWidget::itemActivated, this, &TextCompareTabPage::navigateToDifference);
    connect(m_diffList, &QListWidget::itemClicked, this, &TextCompareTabPage::navigateToDifference);
    connect(m_diffList, &QListWidget::currentRowChanged, this, [this](int) {
        highlightDiffLines();
    });
    connect(m_diffOverviewBar, &TextDiffOverviewBar::differenceBlockActivated, this, [this](int blockIndex) {
        for (int row = 0; row < m_diffList->count(); ++row) {
            auto *item = m_diffList->item(row);
            if (item && item->data(Qt::UserRole + 5).toInt() == blockIndex) {
                selectDifferenceRow(row);
                return;
            }
        }
    });
}

void TextCompareTabPage::loadStartupState()
{
    updateCompareOptionsFromSettings();
    applyDescriptor();

    if (m_inputHeader->leftPathEdit()->text().isEmpty())
        m_leftEditor->setPlainText(QStringLiteral("alpha\nbeta\ngamma\ndelta"));
    if (m_inputHeader->rightPathEdit()->text().isEmpty())
        m_rightEditor->setPlainText(QStringLiteral("alpha\nbeta changed\ngamma\nepsilon"));
}

void TextCompareTabPage::applyDescriptor()
{
    if (!m_descriptor.leftPath.isEmpty() || !m_descriptor.rightPath.isEmpty()) {
        m_inputHeader->leftPathEdit()->setText(m_descriptor.leftPath);
        m_inputHeader->rightPathEdit()->setText(m_descriptor.rightPath);
    }
}

bool TextCompareTabPage::executeCompareFromDescriptorOrPaths(const QString &leftPath, const QString &rightPath)
{
    bool ok = false;
    if (!leftPath.isEmpty() || !rightPath.isEmpty()) {
        ok = m_compareSession.comparePathsOrText(leftPath,
                                                 rightPath,
                                                 QString(),
                                                 QString(),
                                                 leftPath.isEmpty() ? tr("Left") : QString(),
                                                 rightPath.isEmpty() ? tr("Right") : QString());
        return ok;
    }

    return m_compareSession.compareText(m_leftEditor->toPlainText(), m_rightEditor->toPlainText(), tr("Left"), tr("Right"));
}

void TextCompareTabPage::updateCompareOptionsFromSettings()
{
    const auto *compare = mergeqt::app::SettingsService::instance()->compare();
    {
        QSignalBlocker blocker(m_ignoreCaseAction);
        m_ignoreCaseAction->setChecked(compare->ignoreCase());
    }
    {
        QSignalBlocker blocker(m_trimWhitespaceAction);
        m_trimWhitespaceAction->setChecked(compare->trimWhitespace());
    }
    {
        QSignalBlocker blocker(m_showIdenticalItemsAction);
        m_showIdenticalItemsAction->setChecked(compare->showIdenticalItems());
    }
}

void TextCompareTabPage::refreshDocumentMeta()
{
    const auto leftDocument = m_compareSession.currentLeftDocument();
    const auto rightDocument = m_compareSession.currentRightDocument();

    const QString leftLabel = leftDocument.label.isEmpty() ? tr("Left") : leftDocument.label;
    const QString rightLabel = rightDocument.label.isEmpty() ? tr("Right") : rightDocument.label;

    m_leftMetaLabel->setText(tr("%1 | %2 | %3 | %4 lines")
                                 .arg(leftLabel)
                                 .arg(leftDocument.encodingName.isEmpty() ? QStringLiteral("UTF-8") : leftDocument.encodingName)
                                 .arg(newlineStyleText(leftDocument.newlineStyle))
                                 .arg(leftDocument.lines.size()));
    m_rightMetaLabel->setText(tr("%1 | %2 | %3 | %4 lines")
                                  .arg(rightLabel)
                                  .arg(rightDocument.encodingName.isEmpty() ? QStringLiteral("UTF-8") : rightDocument.encodingName)
                                  .arg(newlineStyleText(rightDocument.newlineStyle))
                                  .arg(rightDocument.lines.size()));
}

void TextCompareTabPage::updateAlignedEditors()
{
    const auto result = m_compareSession.currentResult();

    QStringList leftLines;
    QStringList rightLines;
    QVector<int> leftLineNumbers;
    QVector<int> rightLineNumbers;
    leftLines.reserve(result.lines.size());
    rightLines.reserve(result.lines.size());
    leftLineNumbers.reserve(result.lines.size());
    rightLineNumbers.reserve(result.lines.size());

    for (const auto &line : result.lines) {
        leftLines.append(line.leftText);
        rightLines.append(line.rightText);
        leftLineNumbers.append(line.leftLineNumber);
        rightLineNumbers.append(line.rightLineNumber);
    }

    const QSignalBlocker leftBlocker(m_leftEditor);
    const QSignalBlocker rightBlocker(m_rightEditor);
    m_leftEditor->setPlainText(leftLines.join(QLatin1Char('\n')));
    m_rightEditor->setPlainText(rightLines.join(QLatin1Char('\n')));
    m_leftEditor->setDisplayedLineNumbers(leftLineNumbers);
    m_rightEditor->setDisplayedLineNumbers(rightLineNumbers);
}

void TextCompareTabPage::navigateToDifference(QListWidgetItem *item)
{
    if (!item)
        return;

    focusLine(m_leftEditor, item->data(Qt::UserRole).toInt());
    focusLine(m_rightEditor, item->data(Qt::UserRole + 1).toInt());
}

void TextCompareTabPage::selectDifferenceRow(int row)
{
    if (row < 0 || row >= m_diffList->count())
        return;

    m_diffList->setCurrentRow(row);
    m_diffOverviewBar->setSelectedBlockIndex(m_diffList->item(row)->data(Qt::UserRole + 5).toInt());
    navigateToDifference(m_diffList->item(row));
}

void TextCompareTabPage::refreshResultView()
{
    m_diffList->clear();

    const auto result = m_compareSession.currentResult();
    const bool showIdenticalItems = mergeqt::app::SettingsService::instance()->compare()->showIdenticalItems();
    int differenceBlockCount = 0;
    for (const auto &block : result.blocks) {
        if (block.isDifference())
            ++differenceBlockCount;
    }

    m_resultSummaryLabel->setText(
        tr("Difference blocks: %1, difference rows: %2")
            .arg(differenceBlockCount)
            .arg(result.stats.differenceCount()));

    for (int blockIndex = 0; blockIndex < result.blocks.size(); ++blockIndex) {
        const auto &block = result.blocks.at(blockIndex);
        if (!showIdenticalItems && !block.isDifference())
            continue;

        const auto &firstLine = result.lines.at(block.startIndex);
        QString preview = firstLine.leftText;
        if (preview.isEmpty())
            preview = firstLine.rightText;
        if (preview.isEmpty())
            preview = QStringLiteral("<empty>");
        preview = preview.simplified();

        const QString label = tr("%1 | L%2 -> R%3 | %4 row(s) | %5")
                                  .arg(operationText(block.operation))
                                  .arg(blockRangeText(block.firstLeftLineNumber, block.lastLeftLineNumber))
                                  .arg(blockRangeText(block.firstRightLineNumber, block.lastRightLineNumber))
                                  .arg(block.lineCount)
                                  .arg(preview);
        auto *item = new QListWidgetItem(label, m_diffList);
        item->setData(Qt::UserRole, block.startIndex + 1);
        item->setData(Qt::UserRole + 1, block.startIndex + 1);
        item->setData(Qt::UserRole + 2, static_cast<int>(block.operation));
        item->setData(Qt::UserRole + 3, block.startIndex);
        item->setData(Qt::UserRole + 4, block.endIndex);
        item->setData(Qt::UserRole + 5, blockIndex);
    }

    updateAlignedEditors();
    m_diffOverviewBar->setCompareResult(result);

    if (m_diffList->count() == 0) {
        m_diffList->addItem(tr("No differences detected."));
        m_diffOverviewBar->setSelectedBlockIndex(-1);
    } else {
        int firstDifferenceRow = 0;
        while (firstDifferenceRow < m_diffList->count()) {
            const auto *item = m_diffList->item(firstDifferenceRow);
            const bool isSameLine = item->data(Qt::UserRole + 2).toInt()
                                    == static_cast<int>(mergeqt::core::TextDiffOperation::Equal);
            if (!isSameLine)
                break;
            ++firstDifferenceRow;
        }
        selectDifferenceRow(qMin(firstDifferenceRow, m_diffList->count() - 1));
    }

    highlightDiffLines();
    emitPageState();
}

QString TextCompareTabPage::operationText(::mergeqt::core::TextDiffOperation operation) const
{
    switch (operation) {
    case mergeqt::core::TextDiffOperation::Equal:
        return tr("Equal");
    case mergeqt::core::TextDiffOperation::Insert:
        return tr("Insert");
    case mergeqt::core::TextDiffOperation::Delete:
        return tr("Delete");
    case mergeqt::core::TextDiffOperation::Replace:
        return tr("Replace");
    }

    return tr("Unknown");
}

QString TextCompareTabPage::blockRangeText(int firstLine, int lastLine) const
{
    if (firstLine <= 0 && lastLine <= 0)
        return QStringLiteral("-");
    if (firstLine <= 0)
        return QString::number(lastLine);
    if (lastLine <= 0 || firstLine == lastLine)
        return QString::number(firstLine);
    return tr("%1-%2").arg(firstLine).arg(lastLine);
}

void TextCompareTabPage::highlightDiffLines()
{
    QList<QTextEdit::ExtraSelection> leftSelections;
    QList<QTextEdit::ExtraSelection> rightSelections;
    const auto result = m_compareSession.currentResult();
    const int selectedLeftLine = currentDifferenceLeftLine();
    const int selectedRightLine = currentDifferenceRightLine();
    const int selectedStartIndex = currentBlockStartIndex();
    const int selectedEndIndex = currentBlockEndIndex();

    for (int index = 0; index < result.lines.size(); ++index) {
        const auto &line = result.lines.at(index);
        if (!line.isDifference())
            continue;

        const bool inSelectedBlock = selectedStartIndex >= 0
                                     && selectedEndIndex >= selectedStartIndex
                                     && index >= selectedStartIndex
                                     && index <= selectedEndIndex;

        {
            QTextEdit::ExtraSelection selection;
            selection.format.setBackground(inSelectedBlock
                                               ? QColor(255, 213, 79)
                                               : (index + 1 == selectedLeftLine
                                                      ? QColor(255, 193, 7)
                                                      : QColor(255, 236, 179)));
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            QTextBlock block = m_leftEditor->document()->findBlockByLineNumber(index);
            selection.cursor = QTextCursor(block);
            leftSelections.append(selection);
        }

        {
            QTextEdit::ExtraSelection selection;
            selection.format.setBackground(inSelectedBlock
                                               ? QColor(229, 115, 115)
                                               : (index + 1 == selectedRightLine
                                                      ? QColor(239, 83, 80)
                                                      : QColor(255, 205, 210)));
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            QTextBlock block = m_rightEditor->document()->findBlockByLineNumber(index);
            selection.cursor = QTextCursor(block);
            rightSelections.append(selection);
        }
    }

    m_leftEditor->setExtraSelections(leftSelections);
    m_rightEditor->setExtraSelections(rightSelections);
}

void TextCompareTabPage::focusLine(QPlainTextEdit *editor, int lineNumber)
{
    if (!editor || lineNumber <= 0)
        return;

    QTextBlock block = editor->document()->findBlockByLineNumber(lineNumber - 1);
    if (!block.isValid())
        return;

    QTextCursor cursor(block);
    editor->setTextCursor(cursor);
    editor->centerCursor();
}

int TextCompareTabPage::currentDifferenceLeftLine() const
{
    const auto *item = m_diffList->currentItem();
    return item ? item->data(Qt::UserRole).toInt() : -1;
}

int TextCompareTabPage::currentDifferenceRightLine() const
{
    const auto *item = m_diffList->currentItem();
    return item ? item->data(Qt::UserRole + 1).toInt() : -1;
}

int TextCompareTabPage::currentBlockStartIndex() const
{
    const auto *item = m_diffList->currentItem();
    return item ? item->data(Qt::UserRole + 3).toInt() : -1;
}

int TextCompareTabPage::currentBlockEndIndex() const
{
    const auto *item = m_diffList->currentItem();
    return item ? item->data(Qt::UserRole + 4).toInt() : -1;
}

QString TextCompareTabPage::newlineStyleText(::mergeqt::core::NewlineStyle style) const
{
    switch (style) {
    case mergeqt::core::NewlineStyle::None:
        return tr("No newline");
    case mergeqt::core::NewlineStyle::LF:
        return tr("LF");
    case mergeqt::core::NewlineStyle::CRLF:
        return tr("CRLF");
    case mergeqt::core::NewlineStyle::CR:
        return tr("CR");
    case mergeqt::core::NewlineStyle::Mixed:
        return tr("Mixed");
    }

    return tr("Unknown");
}

void TextCompareTabPage::emitPageState()
{
    emit pageTitleChanged(pageTitle());
    emit pageStatusChanged(pageStatusText());
}

} // namespace mergeqt::ui

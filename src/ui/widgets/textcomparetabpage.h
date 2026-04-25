#pragma once

#include "app/compare/comparesessionservice.h"
#include "ui/widgets/comparetabpage.h"
#include "ui/widgets/linenumbertextedit.h"

class QAction;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QPlainTextEdit;
class QScrollBar;
class QPushButton;
class QSplitter;

namespace mergeqt::ui {

class CompareInputHeader;
class TextDiffOverviewBar;

class TextCompareTabPage : public CompareTabPage
{
    Q_OBJECT

public:
    explicit TextCompareTabPage(const mergeqt::app::ComparePageDescriptor &descriptor = {}, QWidget *parent = nullptr);

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
    void retranslateUi() override;

private:
    void buildUi();
    void connectSignals();
    void loadStartupState();
    void applyDescriptor();
    bool executeCompareFromDescriptorOrPaths(const QString &leftPath, const QString &rightPath);
    void updateCompareOptionsFromSettings();
    void refreshDocumentMeta();
    void updateAlignedEditors();
    void navigateToDifference(QListWidgetItem *item);
    void selectDifferenceRow(int row);
    void refreshResultView();
    QString operationText(::mergeqt::core::TextDiffOperation operation) const;
    QString blockRangeText(int firstLine, int lastLine) const;
    void highlightDiffLines();
    void focusLine(QPlainTextEdit *editor, int lineNumber);
    int currentDifferenceLeftLine() const;
    int currentDifferenceRightLine() const;
    int currentBlockStartIndex() const;
    int currentBlockEndIndex() const;
    QString newlineStyleText(::mergeqt::core::NewlineStyle style) const;
    void emitPageState();

    mergeqt::app::CompareSessionService m_compareSession;
    mergeqt::app::ComparePageDescriptor m_descriptor;
    QLabel *m_pageSummaryLabel = nullptr;
    CompareInputHeader *m_inputHeader = nullptr;
    LineNumberTextEdit *m_leftEditor = nullptr;
    LineNumberTextEdit *m_rightEditor = nullptr;
    TextDiffOverviewBar *m_diffOverviewBar = nullptr;
    QSplitter *m_editorSplitter = nullptr;
    QSplitter *m_contentSplitter = nullptr;
    QListWidget *m_diffList = nullptr;
    QLabel *m_resultSummaryLabel = nullptr;
    QLabel *m_leftMetaLabel = nullptr;
    QLabel *m_rightMetaLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QPushButton *m_ignoreCaseButton = nullptr;
    QPushButton *m_trimWhitespaceButton = nullptr;
    QPushButton *m_showIdenticalButton = nullptr;
    QAction *m_ignoreCaseAction = nullptr;
    QAction *m_trimWhitespaceAction = nullptr;
    QAction *m_showIdenticalItemsAction = nullptr;
};

} // namespace mergeqt::ui

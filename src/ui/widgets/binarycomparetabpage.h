#pragma once

#include "app/compare/binarycomparesessionservice.h"
#include "app/compare/comparetypes.h"
#include "ui/widgets/comparetabpage.h"

class QLabel;
class QPlainTextEdit;

namespace mergeqt::ui {

class CompareInputHeader;

class BinaryCompareTabPage : public CompareTabPage
{
    Q_OBJECT

public:
    explicit BinaryCompareTabPage(const mergeqt::app::ComparePageDescriptor &descriptor = {}, QWidget *parent = nullptr);

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
    void refreshResultView();
    void emitPageState();
    QString fileLabelForPath(const QString &path) const;
    QString sizeText(qint64 size) const;

    mergeqt::app::BinaryCompareSessionService m_compareSession;
    mergeqt::app::ComparePageDescriptor m_descriptor;
    QLabel *m_pageSummaryLabel = nullptr;
    CompareInputHeader *m_inputHeader = nullptr;
    QLabel *m_summaryLabel = nullptr;
    QLabel *m_leftMetaLabel = nullptr;
    QLabel *m_rightMetaLabel = nullptr;
    QPlainTextEdit *m_analysisEdit = nullptr;
    QLabel *m_statusLabel = nullptr;
};

} // namespace mergeqt::ui

#pragma once

#include "ui/widgets/comparetabpage.h"

class QLabel;

namespace mergeqt::ui {

class PlaceholderCompareTabPage : public CompareTabPage
{
    Q_OBJECT

public:
    explicit PlaceholderCompareTabPage(mergeqt::app::ComparePageType type, QWidget *parent = nullptr);

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
    QLabel *m_label = nullptr;
    mergeqt::app::ComparePageType m_type;
    QString m_statusText;
};

} // namespace mergeqt::ui

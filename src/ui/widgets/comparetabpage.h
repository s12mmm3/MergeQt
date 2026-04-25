#pragma once

#include "app/compare/comparetypes.h"

#include <QWidget>

namespace mergeqt::ui {

struct CompareTabCapabilities
{
    bool supportsFileSelection = false;
    bool supportsReload = false;
    bool supportsSwap = false;
    bool supportsDifferenceNavigation = false;
    bool supportsExecuteCompare = false;
};

class CompareTabPage : public QWidget
{
    Q_OBJECT

public:
    using QWidget::QWidget;
    ~CompareTabPage() override = default;

    [[nodiscard]] virtual mergeqt::app::ComparePageType pageType() const = 0;
    [[nodiscard]] virtual QString pageTitle() const = 0;
    [[nodiscard]] virtual QString pageStatusText() const = 0;
    [[nodiscard]] virtual CompareTabCapabilities capabilities() const = 0;

    virtual bool chooseLeftFile() = 0;
    virtual bool chooseRightFile() = 0;
    virtual bool reloadInputs() = 0;
    virtual bool swapInputs() = 0;
    virtual bool executeCompare() = 0;
    virtual void navigateDifferenceByOffset(int offset) = 0;
    virtual void applyWindowSettings() = 0;
    virtual void retranslateUi() {}

Q_SIGNALS:
    void pageTitleChanged(const QString &title);
    void pageStatusChanged(const QString &statusText);
    void openCompareRequested(const mergeqt::app::ComparePageDescriptor &descriptor);
};

} // namespace mergeqt::ui

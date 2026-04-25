#pragma once

#include "app/compare/comparetypes.h"

#include <QDialog>

class QComboBox;
class QDialogButtonBox;
class QLineEdit;
class QPushButton;

namespace mergeqt::ui {

class OpenCompareDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OpenCompareDialog(QWidget *parent = nullptr);

    [[nodiscard]] mergeqt::app::ComparePageDescriptor descriptor() const;

private:
    void buildUi();
    void connectSignals();
    void updateLabelsForCurrentType();
    void chooseLeftPath();
    void chooseRightPath();

    QComboBox *m_typeCombo = nullptr;
    QLineEdit *m_titleEdit = nullptr;
    QLineEdit *m_leftPathEdit = nullptr;
    QLineEdit *m_rightPathEdit = nullptr;
    QPushButton *m_leftBrowseButton = nullptr;
    QPushButton *m_rightBrowseButton = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;
};

} // namespace mergeqt::ui

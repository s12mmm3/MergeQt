#pragma once

#include <QDialog>

class QCheckBox;
class QComboBox;
class QLabel;

namespace mergeqt::ui {

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private:
    void buildUi();
    void loadSettings();
    void applySettings();

    QComboBox *m_languageCombo = nullptr;
    QComboBox *m_styleCombo = nullptr;
    QComboBox *m_colorSchemeCombo = nullptr;
    QCheckBox *m_ignoreCaseCheck = nullptr;
    QCheckBox *m_trimWhitespaceCheck = nullptr;
    QCheckBox *m_showIdenticalItemsCheck = nullptr;
    QLabel *m_settingsPathLabel = nullptr;
};

} // namespace mergeqt::ui

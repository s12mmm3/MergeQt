#pragma once

#include <QWidget>

class QLabel;
class QPushButton;

namespace mergeqt::ui {

class WorkspaceStartPage : public QWidget
{
    Q_OBJECT

public:
    explicit WorkspaceStartPage(QWidget *parent = nullptr);
    void retranslateUi();

Q_SIGNALS:
    void openCompareRequested();
    void newTextCompareRequested();
    void newFolderCompareRequested();
    void newBinaryCompareRequested();
    void newImageCompareRequested();
    void settingsRequested();
    void aboutRequested();

private:
    void buildUi();

    QLabel *m_titleLabel = nullptr;
    QLabel *m_summaryLabel = nullptr;
    QLabel *m_supportTitleLabel = nullptr;
    QLabel *m_tipsLabel = nullptr;
    QPushButton *m_openCompareButton = nullptr;
    QPushButton *m_newTextCompareButton = nullptr;
    QPushButton *m_newFolderCompareButton = nullptr;
    QPushButton *m_newBinaryCompareButton = nullptr;
    QPushButton *m_newImageCompareButton = nullptr;
    QPushButton *m_settingsButton = nullptr;
    QPushButton *m_aboutButton = nullptr;
};

} // namespace mergeqt::ui

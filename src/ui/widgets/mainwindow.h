#pragma once

#include "app/compare/comparetypes.h"
#include "ui/frontend/uifrontend.h"

#include <QMainWindow>
#include <optional>

class QAction;
class QMenu;
class QStackedWidget;
class QTabWidget;
class QToolBar;

namespace mergeqt::ui {

class CompareTabPage;
class WorkspaceStartPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(FrontendWindowHost *windowHost = nullptr,
                        std::optional<mergeqt::app::ComparePageDescriptor> initialDescriptor = std::nullopt,
                        QWidget *parent = nullptr);

private:
    void buildUi();
    void connectSignals();
    void syncSettingsFromWindowSize();
    void applySettingsToInterface(bool languageChanged);
    void retranslateUi();
    void refreshStartPage();
    void updateWorkspaceView();
    void updateActionsForCurrentTab();
    CompareTabPage *currentComparePage() const;
    CompareTabPage *createComparePage(const mergeqt::app::ComparePageDescriptor &descriptor) const;
    void openCompareTab(const mergeqt::app::ComparePageDescriptor &descriptor);
    void handleOpenCompareRequest(const mergeqt::app::ComparePageDescriptor &descriptor);
    void updateCurrentTabTitle();
    void updateStatusFromCurrentTab();

    void newWindow();
    void showStartPage();
    void showTabWorkspace();
    void openCompareDialog();
    void openSettingsDialog();
    void showAboutDialog();
    void newTextCompareTab();
    void newFolderCompareTab();
    void newBinaryCompareTab();
    void newImageCompareTab();
    void closeCurrentTab();
    void chooseLeftFile();
    void chooseRightFile();
    void compareCurrentTab();
    void reloadCurrentTab();
    void swapCurrentTab();
    void navigateCurrentTabByOffset(int offset);

    void resizeEvent(QResizeEvent *event) override;

    QStackedWidget *m_workspaceStack = nullptr;
    WorkspaceStartPage *m_startPage = nullptr;
    QTabWidget *m_tabWidget = nullptr;
    QAction *m_showStartPageAction = nullptr;
    QAction *m_showTabsAction = nullptr;
    QAction *m_openCompareAction = nullptr;
    QAction *m_newWindowAction = nullptr;
    QAction *m_newTextTabAction = nullptr;
    QAction *m_newFolderTabAction = nullptr;
    QAction *m_newBinaryTabAction = nullptr;
    QAction *m_newImageTabAction = nullptr;
    QAction *m_closeTabAction = nullptr;
    QAction *m_chooseLeftAction = nullptr;
    QAction *m_chooseRightAction = nullptr;
    QAction *m_compareAction = nullptr;
    QAction *m_reloadAction = nullptr;
    QAction *m_swapSidesAction = nullptr;
    QAction *m_nextDifferenceAction = nullptr;
    QAction *m_previousDifferenceAction = nullptr;
    QAction *m_settingsAction = nullptr;
    QAction *m_aboutAction = nullptr;
    QMenu *m_fileMenu = nullptr;
    QMenu *m_navigateMenu = nullptr;
    QMenu *m_toolsMenu = nullptr;
    QMenu *m_workspaceMenu = nullptr;
    QMenu *m_helpMenu = nullptr;
    QToolBar *m_workspaceToolBar = nullptr;
    FrontendWindowHost *m_windowHost = nullptr;
    std::optional<mergeqt::app::ComparePageDescriptor> m_initialDescriptor;
    bool m_preferStartPage = true;
};

} // namespace mergeqt::ui

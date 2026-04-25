#include "ui/widgets/applicationappearance.h"
#include "ui/widgets/mainwindow.h"

#include "app/application/appcontext.h"
#include "app/settings/settingsservice.h"
#include "ui/widgets/comparepagefactory.h"
#include "ui/widgets/comparetabpage.h"
#include "ui/widgets/opencomparedialog.h"
#include "ui/widgets/settingsdialog.h"
#include "ui/widgets/workspacestartpage.h"

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QGuiApplication>
#include <QMenuBar>
#include <QMessageBox>
#include <QResizeEvent>
#include <QStatusBar>
#include <QStackedWidget>
#include <QStyle>
#include <QStyleHints>
#include <QStyleFactory>
#include <QTabWidget>
#include <QToolBar>
#include <QVBoxLayout>

namespace mergeqt::ui {

MainWindow::MainWindow(FrontendWindowHost *windowHost,
                       std::optional<mergeqt::app::ComparePageDescriptor> initialDescriptor,
                       QWidget *parent)
    : QMainWindow(parent)
    , m_windowHost(windowHost)
    , m_initialDescriptor(std::move(initialDescriptor))
{
    auto *settings = mergeqt::app::SettingsService::instance();
    resize(settings->ui()->windowWidth(), settings->ui()->windowHeight());
    setMinimumSize(1024, 700);
    setAttribute(Qt::WA_DeleteOnClose, true);
    buildUi();
    connectSignals();
    retranslateUi();
    refreshStartPage();
    updateWorkspaceView();
    updateActionsForCurrentTab();
    updateStatusFromCurrentTab();

    if (m_initialDescriptor.has_value())
        openCompareTab(*m_initialDescriptor);
}

void MainWindow::buildUi()
{
    m_showStartPageAction = new QAction(this);
    m_showStartPageAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_1));
    m_showTabsAction = new QAction(this);
    m_showTabsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_2));
    m_openCompareAction = new QAction(this);
    m_openCompareAction->setShortcut(QKeySequence::Open);
    m_newWindowAction = new QAction(this);
    m_newWindowAction->setShortcut(QKeySequence::New);
    m_newTextTabAction = new QAction(this);
    m_newTextTabAction->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_T));
    m_newFolderTabAction = new QAction(this);
    m_newBinaryTabAction = new QAction(this);
    m_newImageTabAction = new QAction(this);
    m_closeTabAction = new QAction(this);
    m_closeTabAction->setShortcut(QKeySequence::Close);
    m_chooseLeftAction = new QAction(this);
    m_chooseRightAction = new QAction(this);
    m_compareAction = new QAction(this);
    m_compareAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
    m_reloadAction = new QAction(this);
    m_reloadAction->setShortcut(QKeySequence::Refresh);
    m_swapSidesAction = new QAction(this);
    m_swapSidesAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
    m_previousDifferenceAction = new QAction(this);
    m_previousDifferenceAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F7));
    m_nextDifferenceAction = new QAction(this);
    m_nextDifferenceAction->setShortcut(QKeySequence(Qt::Key_F7));
    m_settingsAction = new QAction(this);
    m_settingsAction->setShortcut(QKeySequence::Preferences);
    m_aboutAction = new QAction(this);

    m_fileMenu = menuBar()->addMenu(QString());
    m_fileMenu->addAction(m_newWindowAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_showStartPageAction);
    m_fileMenu->addAction(m_showTabsAction);
    m_fileMenu->addAction(m_openCompareAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_newTextTabAction);
    m_fileMenu->addAction(m_newFolderTabAction);
    m_fileMenu->addAction(m_newBinaryTabAction);
    m_fileMenu->addAction(m_newImageTabAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_chooseLeftAction);
    m_fileMenu->addAction(m_chooseRightAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_compareAction);
    m_fileMenu->addAction(m_reloadAction);
    m_fileMenu->addAction(m_swapSidesAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_closeTabAction);

    m_navigateMenu = menuBar()->addMenu(QString());
    m_navigateMenu->addAction(m_previousDifferenceAction);
    m_navigateMenu->addAction(m_nextDifferenceAction);

    m_toolsMenu = menuBar()->addMenu(QString());
    m_toolsMenu->addAction(m_settingsAction);

    m_workspaceMenu = menuBar()->addMenu(QString());
    m_workspaceMenu->addAction(m_showStartPageAction);
    m_workspaceMenu->addAction(m_showTabsAction);
    m_workspaceMenu->addAction(m_openCompareAction);
    m_workspaceMenu->addSeparator();
    m_workspaceMenu->addAction(m_newTextTabAction);
    m_workspaceMenu->addAction(m_newFolderTabAction);
    m_workspaceMenu->addAction(m_newBinaryTabAction);
    m_workspaceMenu->addAction(m_newImageTabAction);

    m_helpMenu = menuBar()->addMenu(QString());
    m_helpMenu->addAction(m_aboutAction);

    m_workspaceToolBar = addToolBar(QString());
    m_workspaceToolBar->setMovable(false);
    m_workspaceToolBar->setFloatable(false);
    m_workspaceToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_workspaceToolBar->addAction(m_openCompareAction);
    m_workspaceToolBar->addSeparator();
    m_workspaceToolBar->addAction(m_newTextTabAction);
    m_workspaceToolBar->addAction(m_newFolderTabAction);
    m_workspaceToolBar->addSeparator();
    m_workspaceToolBar->addAction(m_chooseLeftAction);
    m_workspaceToolBar->addAction(m_chooseRightAction);
    m_workspaceToolBar->addAction(m_compareAction);
    m_workspaceToolBar->addAction(m_reloadAction);
    m_workspaceToolBar->addAction(m_swapSidesAction);
    m_workspaceToolBar->addSeparator();
    m_workspaceToolBar->addAction(m_previousDifferenceAction);
    m_workspaceToolBar->addAction(m_nextDifferenceAction);

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_workspaceStack = new QStackedWidget(central);
    m_startPage = new WorkspaceStartPage(m_workspaceStack);
    m_tabWidget = new QTabWidget(m_workspaceStack);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setMovable(true);
    m_workspaceStack->addWidget(m_startPage);
    m_workspaceStack->addWidget(m_tabWidget);
    layout->addWidget(m_workspaceStack);

    statusBar()->setSizeGripEnabled(false);
    statusBar()->showMessage(tr("Open a compare tab or use Open Compare to choose files or folders."));
    setCentralWidget(central);
}

void MainWindow::connectSignals()
{
    connect(m_startPage, &WorkspaceStartPage::openCompareRequested, this, &MainWindow::openCompareDialog);
    connect(m_startPage, &WorkspaceStartPage::newTextCompareRequested, this, &MainWindow::newTextCompareTab);
    connect(m_startPage, &WorkspaceStartPage::newFolderCompareRequested, this, &MainWindow::newFolderCompareTab);
    connect(m_startPage, &WorkspaceStartPage::newBinaryCompareRequested, this, &MainWindow::newBinaryCompareTab);
    connect(m_startPage, &WorkspaceStartPage::newImageCompareRequested, this, &MainWindow::newImageCompareTab);
    connect(m_startPage, &WorkspaceStartPage::settingsRequested, this, &MainWindow::openSettingsDialog);
    connect(m_startPage, &WorkspaceStartPage::aboutRequested, this, &MainWindow::showAboutDialog);
    connect(m_showStartPageAction, &QAction::triggered, this, &MainWindow::showStartPage);
    connect(m_showTabsAction, &QAction::triggered, this, &MainWindow::showTabWorkspace);
    connect(m_openCompareAction, &QAction::triggered, this, &MainWindow::openCompareDialog);
    connect(m_newWindowAction, &QAction::triggered, this, &MainWindow::newWindow);
    connect(m_newTextTabAction, &QAction::triggered, this, &MainWindow::newTextCompareTab);
    connect(m_newFolderTabAction, &QAction::triggered, this, &MainWindow::newFolderCompareTab);
    connect(m_newBinaryTabAction, &QAction::triggered, this, &MainWindow::newBinaryCompareTab);
    connect(m_newImageTabAction, &QAction::triggered, this, &MainWindow::newImageCompareTab);
    connect(m_closeTabAction, &QAction::triggered, this, &MainWindow::closeCurrentTab);
    connect(m_chooseLeftAction, &QAction::triggered, this, &MainWindow::chooseLeftFile);
    connect(m_chooseRightAction, &QAction::triggered, this, &MainWindow::chooseRightFile);
    connect(m_compareAction, &QAction::triggered, this, &MainWindow::compareCurrentTab);
    connect(m_reloadAction, &QAction::triggered, this, &MainWindow::reloadCurrentTab);
    connect(m_swapSidesAction, &QAction::triggered, this, &MainWindow::swapCurrentTab);
    connect(m_previousDifferenceAction, &QAction::triggered, this, [this]() { navigateCurrentTabByOffset(-1); });
    connect(m_nextDifferenceAction, &QAction::triggered, this, [this]() { navigateCurrentTabByOffset(1); });
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::openSettingsDialog);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);
    connect(mergeqt::app::AppContext::instance(), &mergeqt::app::AppContext::currentLanguageChanged, this, [this]() {
        retranslateUi();
        refreshStartPage();
        updateCurrentTabTitle();
        updateStatusFromCurrentTab();
    });

    connect(m_tabWidget, &QTabWidget::currentChanged, this, [this](int) {
        if (auto *page = currentComparePage())
            page->applyWindowSettings();
        updateActionsForCurrentTab();
        updateCurrentTabTitle();
        updateStatusFromCurrentTab();
    });
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, [this](int index) {
        QWidget *page = m_tabWidget->widget(index);
        m_tabWidget->removeTab(index);
        delete page;
        updateWorkspaceView();
        updateActionsForCurrentTab();
        updateCurrentTabTitle();
        updateStatusFromCurrentTab();
    });
}

void MainWindow::syncSettingsFromWindowSize()
{
    auto *ui = mergeqt::app::SettingsService::instance()->ui();
    ui->set_windowWidth(width());
    ui->set_windowHeight(height());
}

void MainWindow::applySettingsToInterface(bool languageChanged)
{
    const auto *uiSettings = mergeqt::app::SettingsService::instance()->ui();
    applyApplicationAppearance();
    if (!isMaximized() && !isFullScreen()
        && (width() != uiSettings->windowWidth() || height() != uiSettings->windowHeight()))
        resize(uiSettings->windowWidth(), uiSettings->windowHeight());

    for (int index = 0; index < m_tabWidget->count(); ++index) {
        if (auto *page = qobject_cast<CompareTabPage *>(m_tabWidget->widget(index))) {
            page->applyWindowSettings();
            if (languageChanged)
                page->retranslateUi();
        }
    }

    refreshStartPage();
    updateWorkspaceView();
    updateActionsForCurrentTab();
    updateCurrentTabTitle();
    updateStatusFromCurrentTab();

    if (languageChanged)
        statusBar()->showMessage(tr("Language changed."), 3000);
}

void MainWindow::retranslateUi()
{
    if (m_showStartPageAction)
        m_showStartPageAction->setText(tr("Workspace Home"));
    if (m_showTabsAction)
        m_showTabsAction->setText(tr("Open Tabs"));
    if (m_openCompareAction)
        m_openCompareAction->setText(tr("Open Compare..."));
    if (m_newWindowAction)
        m_newWindowAction->setText(tr("New Window"));
    if (m_newTextTabAction)
        m_newTextTabAction->setText(tr("New Text Compare Tab"));
    if (m_newFolderTabAction)
        m_newFolderTabAction->setText(tr("New Folder Compare Tab"));
    if (m_newBinaryTabAction)
        m_newBinaryTabAction->setText(tr("New Binary Compare Tab"));
    if (m_newImageTabAction)
        m_newImageTabAction->setText(tr("New Image Compare Tab"));
    if (m_closeTabAction)
        m_closeTabAction->setText(tr("Close Tab"));
    if (m_chooseLeftAction)
        m_chooseLeftAction->setText(tr("Open Left..."));
    if (m_chooseRightAction)
        m_chooseRightAction->setText(tr("Open Right..."));
    if (m_compareAction)
        m_compareAction->setText(tr("Compare"));
    if (m_reloadAction)
        m_reloadAction->setText(tr("Reload Files"));
    if (m_swapSidesAction)
        m_swapSidesAction->setText(tr("Swap Sides"));
    if (m_previousDifferenceAction)
        m_previousDifferenceAction->setText(tr("Previous Difference"));
    if (m_nextDifferenceAction)
        m_nextDifferenceAction->setText(tr("Next Difference"));
    if (m_settingsAction)
        m_settingsAction->setText(tr("Settings..."));
    if (m_aboutAction)
        m_aboutAction->setText(tr("About MergeQt"));
    if (m_fileMenu)
        m_fileMenu->setTitle(tr("&File"));
    if (m_navigateMenu)
        m_navigateMenu->setTitle(tr("&Navigate"));
    if (m_toolsMenu)
        m_toolsMenu->setTitle(tr("&Tools"));
    if (m_workspaceMenu)
        m_workspaceMenu->setTitle(tr("&Workspace"));
    if (m_helpMenu)
        m_helpMenu->setTitle(tr("&Help"));
    if (m_workspaceToolBar)
        m_workspaceToolBar->setWindowTitle(tr("Workspace"));
    if (m_startPage)
        m_startPage->retranslateUi();
    for (int index = 0; index < m_tabWidget->count(); ++index) {
        if (auto *page = qobject_cast<CompareTabPage *>(m_tabWidget->widget(index)))
            page->retranslateUi();
    }
}

void MainWindow::refreshStartPage()
{
    if (!m_startPage)
        return;
}

void MainWindow::updateWorkspaceView()
{
    if (!m_workspaceStack || !m_startPage || !m_tabWidget)
        return;

    const bool hasTabs = m_tabWidget->count() > 0;
    if (!hasTabs)
        m_preferStartPage = true;

    m_workspaceStack->setCurrentWidget((m_preferStartPage || !hasTabs) ? static_cast<QWidget *>(m_startPage)
                                                                       : static_cast<QWidget *>(m_tabWidget));
    if (m_workspaceToolBar)
        m_workspaceToolBar->setVisible(m_workspaceStack->currentWidget() == m_tabWidget);
}

void MainWindow::updateActionsForCurrentTab()
{
    const auto *page = currentComparePage();
    const bool isWorkspaceTabVisible = m_workspaceStack && m_workspaceStack->currentWidget() == m_tabWidget;
    const bool hasTab = isWorkspaceTabVisible && page != nullptr && m_tabWidget->count() > 0;
    const CompareTabCapabilities caps = page ? page->capabilities() : CompareTabCapabilities{};
    m_showStartPageAction->setEnabled(!(m_workspaceStack && m_workspaceStack->currentWidget() == m_startPage && m_tabWidget->count() == 0));
    m_showTabsAction->setEnabled(m_tabWidget->count() > 0 && (!m_workspaceStack || m_workspaceStack->currentWidget() != m_tabWidget));
    m_closeTabAction->setEnabled(hasTab && m_tabWidget->count() > 0);
    m_chooseLeftAction->setEnabled(hasTab && caps.supportsFileSelection);
    m_chooseRightAction->setEnabled(hasTab && caps.supportsFileSelection);
    m_compareAction->setEnabled(hasTab && caps.supportsExecuteCompare);
    m_reloadAction->setEnabled(hasTab && caps.supportsReload);
    m_swapSidesAction->setEnabled(hasTab && caps.supportsSwap);
    m_previousDifferenceAction->setEnabled(hasTab && caps.supportsDifferenceNavigation);
    m_nextDifferenceAction->setEnabled(hasTab && caps.supportsDifferenceNavigation);
}

CompareTabPage *MainWindow::currentComparePage() const
{
    return qobject_cast<CompareTabPage *>(m_tabWidget->currentWidget());
}

CompareTabPage *MainWindow::createComparePage(const mergeqt::app::ComparePageDescriptor &descriptor) const
{
    return mergeqt::ui::createComparePage(descriptor);
}

void MainWindow::openCompareTab(const mergeqt::app::ComparePageDescriptor &descriptor)
{
    CompareTabPage *page = createComparePage(descriptor);
    const int index = m_tabWidget->addTab(page, descriptor.title.isEmpty() ? page->pageTitle() : descriptor.title);
    m_tabWidget->setCurrentIndex(index);
    refreshStartPage();
    m_preferStartPage = false;
    updateWorkspaceView();
    page->applyWindowSettings();

    connect(page, &CompareTabPage::pageTitleChanged, this, [this, page](const QString &title) {
        const int tabIndex = m_tabWidget->indexOf(page);
        if (tabIndex >= 0)
            m_tabWidget->setTabText(tabIndex, title);
        updateCurrentTabTitle();
    });
    connect(page, &CompareTabPage::pageStatusChanged, this, [this](const QString &) {
        updateStatusFromCurrentTab();
    });
    connect(page, &CompareTabPage::openCompareRequested, this, &MainWindow::handleOpenCompareRequest);
}

void MainWindow::handleOpenCompareRequest(const mergeqt::app::ComparePageDescriptor &descriptor)
{
    openCompareTab(descriptor);
}

void MainWindow::updateCurrentTabTitle()
{
    const bool showStartPageNow = !m_workspaceStack || m_workspaceStack->currentWidget() == m_startPage;
    const auto *page = showStartPageNow ? nullptr : currentComparePage();
    const QString title = page ? page->pageTitle() : tr("Start Page");
    setWindowTitle(tr("MergeQt - %1").arg(title));
}

void MainWindow::updateStatusFromCurrentTab()
{
    const bool showStartPageNow = !m_workspaceStack || m_workspaceStack->currentWidget() == m_startPage;
    const auto *page = showStartPageNow ? nullptr : currentComparePage();
    const QString status = page ? page->pageStatusText()
                                : tr("Choose a compare type from the start page or open a compare directly.");
    statusBar()->showMessage(status);
}

void MainWindow::newWindow()
{
    if (m_windowHost) {
        m_windowHost->openMainWindow();
        return;
    }

    auto *window = new MainWindow(nullptr, std::nullopt);
    window->show();
}

void MainWindow::showStartPage()
{
    m_preferStartPage = true;
    refreshStartPage();
    updateWorkspaceView();
    updateActionsForCurrentTab();
    updateCurrentTabTitle();
    updateStatusFromCurrentTab();
}

void MainWindow::showTabWorkspace()
{
    if (!m_tabWidget || m_tabWidget->count() <= 0)
        return;

    m_preferStartPage = false;
    updateWorkspaceView();
    updateActionsForCurrentTab();
    updateCurrentTabTitle();
    updateStatusFromCurrentTab();
}

void MainWindow::openCompareDialog()
{
    OpenCompareDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    auto descriptor = dialog.descriptor();
    if (descriptor.title.isEmpty())
        descriptor.title = mergeqt::ui::defaultComparePageTitle(descriptor.type, this);

    openCompareTab(descriptor);
}

void MainWindow::openSettingsDialog()
{
    auto *settings = mergeqt::app::SettingsService::instance();
    const QString previousLanguage = settings->ui()->language();
    SettingsDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    if (settings->ui()->language() != previousLanguage)
        mergeqt::app::AppContext::instance()->switchLanguage(settings->ui()->language());

    applySettingsToInterface(settings->ui()->language() != previousLanguage);
}

void MainWindow::showAboutDialog()
{
    QMessageBox::about(this, tr("About MergeQt"),
                       tr("MergeQt is a Qt Widgets-based desktop diff and merge workbench.\n\n"
                          "Current focus:\n"
                          "- text compare with aligned rows and original line numbers\n"
                          "- multi-tab and multi-window workspace shell\n"
                          "- extensible architecture for folder, binary, and image compare"));
}

void MainWindow::newTextCompareTab()
{
    openCompareTab({mergeqt::app::ComparePageType::Text, tr("Text Compare")});
}

void MainWindow::newFolderCompareTab()
{
    openCompareTab({mergeqt::app::ComparePageType::Folder, tr("Folder Compare")});
}

void MainWindow::newBinaryCompareTab()
{
    openCompareTab({mergeqt::app::ComparePageType::Binary, tr("Binary Compare")});
}

void MainWindow::newImageCompareTab()
{
    openCompareTab({mergeqt::app::ComparePageType::Image, tr("Image Compare")});
}

void MainWindow::closeCurrentTab()
{
    const int index = m_tabWidget->currentIndex();
    if (index < 0)
        return;
    QWidget *page = m_tabWidget->widget(index);
    m_tabWidget->removeTab(index);
    delete page;
    updateWorkspaceView();
    updateActionsForCurrentTab();
    updateCurrentTabTitle();
    updateStatusFromCurrentTab();
}

void MainWindow::chooseLeftFile()
{
    if (auto *page = currentComparePage())
        page->chooseLeftFile();
}

void MainWindow::chooseRightFile()
{
    if (auto *page = currentComparePage())
        page->chooseRightFile();
}

void MainWindow::compareCurrentTab()
{
    if (auto *page = currentComparePage())
        page->executeCompare();
}

void MainWindow::reloadCurrentTab()
{
    if (auto *page = currentComparePage())
        page->reloadInputs();
}

void MainWindow::swapCurrentTab()
{
    if (auto *page = currentComparePage())
        page->swapInputs();
}

void MainWindow::navigateCurrentTabByOffset(int offset)
{
    if (auto *page = currentComparePage())
        page->navigateDifferenceByOffset(offset);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    syncSettingsFromWindowSize();
}

} // namespace mergeqt::ui

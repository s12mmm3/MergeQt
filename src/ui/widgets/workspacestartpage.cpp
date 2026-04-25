#include "ui/widgets/workspacestartpage.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace mergeqt::ui {

WorkspaceStartPage::WorkspaceStartPage(QWidget *parent)
    : QWidget(parent)
{
    buildUi();
    retranslateUi();
}

void WorkspaceStartPage::buildUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(36, 36, 36, 36);
    rootLayout->setSpacing(18);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName(QStringLiteral("workspaceStartTitle"));
    m_titleLabel->setStyleSheet(QStringLiteral("#workspaceStartTitle { font-size: 28px; font-weight: 700; }"));
    rootLayout->addWidget(m_titleLabel);

    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setWordWrap(true);
    rootLayout->addWidget(m_summaryLabel);

    auto *buttonRow = new QWidget(this);
    auto *buttonLayout = new QHBoxLayout(buttonRow);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(12);

    m_openCompareButton = new QPushButton(buttonRow);
    m_newTextCompareButton = new QPushButton(buttonRow);
    m_newFolderCompareButton = new QPushButton(buttonRow);
    m_newBinaryCompareButton = new QPushButton(buttonRow);
    m_newImageCompareButton = new QPushButton(buttonRow);
    buttonLayout->addWidget(m_openCompareButton);
    buttonLayout->addWidget(m_newTextCompareButton);
    buttonLayout->addWidget(m_newFolderCompareButton);
    buttonLayout->addWidget(m_newBinaryCompareButton);
    buttonLayout->addWidget(m_newImageCompareButton);
    buttonLayout->addStretch(1);

    rootLayout->addWidget(buttonRow);

    m_supportTitleLabel = new QLabel(this);
    m_supportTitleLabel->setObjectName(QStringLiteral("workspaceStartSectionTitle"));
    m_supportTitleLabel->setStyleSheet(QStringLiteral("#workspaceStartSectionTitle { font-size: 16px; font-weight: 600; }"));
    rootLayout->addWidget(m_supportTitleLabel);

    auto *supportRow = new QWidget(this);
    auto *supportLayout = new QHBoxLayout(supportRow);
    supportLayout->setContentsMargins(0, 0, 0, 0);
    supportLayout->setSpacing(12);
    m_settingsButton = new QPushButton(supportRow);
    m_aboutButton = new QPushButton(supportRow);
    supportLayout->addWidget(m_settingsButton);
    supportLayout->addWidget(m_aboutButton);
    supportLayout->addStretch(1);
    rootLayout->addWidget(supportRow);

    m_tipsLabel = new QLabel(this);
    m_tipsLabel->setWordWrap(true);
    rootLayout->addWidget(m_tipsLabel);
    rootLayout->addStretch(1);

    connect(m_openCompareButton, &QPushButton::clicked, this, &WorkspaceStartPage::openCompareRequested);
    connect(m_newTextCompareButton, &QPushButton::clicked, this, &WorkspaceStartPage::newTextCompareRequested);
    connect(m_newFolderCompareButton, &QPushButton::clicked, this, &WorkspaceStartPage::newFolderCompareRequested);
    connect(m_newBinaryCompareButton, &QPushButton::clicked, this, &WorkspaceStartPage::newBinaryCompareRequested);
    connect(m_newImageCompareButton, &QPushButton::clicked, this, &WorkspaceStartPage::newImageCompareRequested);
    connect(m_settingsButton, &QPushButton::clicked, this, &WorkspaceStartPage::settingsRequested);
    connect(m_aboutButton, &QPushButton::clicked, this, &WorkspaceStartPage::aboutRequested);
}

void WorkspaceStartPage::retranslateUi()
{
    if (m_titleLabel)
        m_titleLabel->setText(tr("MergeQt Workspace"));
    if (m_summaryLabel) {
        m_summaryLabel->setText(
            tr("Start from a unified compare dialog, or jump directly into a new text, folder, binary, or image comparison tab."));
    }
    if (m_openCompareButton)
        m_openCompareButton->setText(tr("Open Compare"));
    if (m_newTextCompareButton)
        m_newTextCompareButton->setText(tr("New Text Compare"));
    if (m_newFolderCompareButton)
        m_newFolderCompareButton->setText(tr("New Folder Compare"));
    if (m_newBinaryCompareButton)
        m_newBinaryCompareButton->setText(tr("New Binary Compare"));
    if (m_newImageCompareButton)
        m_newImageCompareButton->setText(tr("New Image Compare"));
    if (m_supportTitleLabel)
        m_supportTitleLabel->setText(tr("Application"));
    if (m_settingsButton)
        m_settingsButton->setText(tr("Settings"));
    if (m_aboutButton)
        m_aboutButton->setText(tr("About"));
    if (m_tipsLabel) {
        m_tipsLabel->setText(
            tr("Use the toolbar and menus to switch between the workspace home page and your open compare tabs. Each Open Compare action starts with a fresh dialog."));
    }
}

} // namespace mergeqt::ui

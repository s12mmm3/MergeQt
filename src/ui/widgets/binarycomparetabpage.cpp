#include "ui/widgets/binarycomparetabpage.h"

#include "ui/widgets/compareinputheader.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

namespace mergeqt::ui {

BinaryCompareTabPage::BinaryCompareTabPage(const mergeqt::app::ComparePageDescriptor &descriptor, QWidget *parent)
    : CompareTabPage(parent)
    , m_compareSession(this)
    , m_descriptor(descriptor)
{
    buildUi();
    connectSignals();
    loadStartupState();
}

mergeqt::app::ComparePageType BinaryCompareTabPage::pageType() const
{
    return mergeqt::app::ComparePageType::Binary;
}

QString BinaryCompareTabPage::pageTitle() const
{
    const QString left = fileLabelForPath(m_inputHeader ? m_inputHeader->leftPathEdit()->text().trimmed() : QString());
    const QString right = fileLabelForPath(m_inputHeader ? m_inputHeader->rightPathEdit()->text().trimmed() : QString());
    return tr("%1 vs %2").arg(left, right);
}

QString BinaryCompareTabPage::pageStatusText() const
{
    return m_statusLabel ? m_statusLabel->text() : QString();
}

CompareTabCapabilities BinaryCompareTabPage::capabilities() const
{
    return { true, true, true, false, true };
}

bool BinaryCompareTabPage::chooseLeftFile()
{
    const QString path =
        QFileDialog::getOpenFileName(this, tr("Open left binary file"), m_inputHeader->leftPathEdit()->text());
    if (path.isEmpty())
        return false;

    m_inputHeader->leftPathEdit()->setText(path);
    return executeCompare();
}

bool BinaryCompareTabPage::chooseRightFile()
{
    const QString path =
        QFileDialog::getOpenFileName(this, tr("Open right binary file"), m_inputHeader->rightPathEdit()->text());
    if (path.isEmpty())
        return false;

    m_inputHeader->rightPathEdit()->setText(path);
    return executeCompare();
}

bool BinaryCompareTabPage::reloadInputs()
{
    const bool ok = m_compareSession.reloadCurrentFiles();
    if (ok)
        emitPageState();
    return ok;
}

bool BinaryCompareTabPage::swapInputs()
{
    if (!m_compareSession.swapInputs())
        return false;

    const auto result = m_compareSession.currentResult();
    m_inputHeader->leftPathEdit()->setText(result.leftPath);
    m_inputHeader->rightPathEdit()->setText(result.rightPath);
    emitPageState();
    return true;
}

bool BinaryCompareTabPage::executeCompare()
{
    if (!m_compareSession.compareFiles(m_inputHeader->leftPathEdit()->text().trimmed(),
                                       m_inputHeader->rightPathEdit()->text().trimmed()))
        return false;

    emitPageState();
    return true;
}

void BinaryCompareTabPage::navigateDifferenceByOffset(int)
{
}

void BinaryCompareTabPage::applyWindowSettings()
{
}

void BinaryCompareTabPage::retranslateUi()
{
    if (m_pageSummaryLabel) {
        m_pageSummaryLabel->setText(
            tr("Use this page to compare two binary files by size and byte-level identity. This is the minimal binary compare shell that later byte diff, hex view, and block navigation features can grow from."));
    }
    if (m_inputHeader)
        m_inputHeader->setLabelTexts(tr("Left file"), tr("Right file"));
    if (m_inputHeader)
        m_inputHeader->retranslateUi();
    if (m_summaryLabel && m_summaryLabel->text().isEmpty())
        m_summaryLabel->setText(tr("Choose two files before running binary compare."));
    if (m_analysisEdit)
        m_analysisEdit->setPlaceholderText(tr("Binary compare summary will appear here."));
    if (m_statusLabel && m_statusLabel->text().isEmpty())
        m_statusLabel->setText(m_summaryLabel ? m_summaryLabel->text() : tr("Choose two files before running binary compare."));
    refreshResultView();
}

void BinaryCompareTabPage::buildUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(12, 12, 12, 12);
    rootLayout->setSpacing(10);

    m_pageSummaryLabel = new QLabel(this);
    m_pageSummaryLabel->setWordWrap(true);
    rootLayout->addWidget(m_pageSummaryLabel);

    m_inputHeader = new CompareInputHeader(QString(), QString(), this);
    rootLayout->addWidget(m_inputHeader);

    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setWordWrap(true);
    rootLayout->addWidget(m_summaryLabel);

    auto *metaRow = new QWidget(this);
    auto *metaLayout = new QHBoxLayout(metaRow);
    metaLayout->setContentsMargins(0, 0, 0, 0);
    m_leftMetaLabel = new QLabel(metaRow);
    m_rightMetaLabel = new QLabel(metaRow);
    m_leftMetaLabel->setWordWrap(true);
    m_rightMetaLabel->setWordWrap(true);
    metaLayout->addWidget(m_leftMetaLabel, 1);
    metaLayout->addWidget(m_rightMetaLabel, 1);
    rootLayout->addWidget(metaRow);

    m_analysisEdit = new QPlainTextEdit(this);
    m_analysisEdit->setReadOnly(true);
    rootLayout->addWidget(m_analysisEdit, 1);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    rootLayout->addWidget(m_statusLabel);

    connect(m_inputHeader, &CompareInputHeader::leftBrowseRequested, this, &BinaryCompareTabPage::chooseLeftFile);
    connect(m_inputHeader, &CompareInputHeader::rightBrowseRequested, this, &BinaryCompareTabPage::chooseRightFile);
    connect(m_inputHeader, &CompareInputHeader::compareRequested, this, &BinaryCompareTabPage::executeCompare);
    connect(m_inputHeader, &CompareInputHeader::reloadRequested, this, &BinaryCompareTabPage::reloadInputs);
    connect(m_inputHeader, &CompareInputHeader::swapRequested, this, &BinaryCompareTabPage::swapInputs);
    retranslateUi();
}

void BinaryCompareTabPage::connectSignals()
{
    connect(&m_compareSession, &mergeqt::app::BinaryCompareSessionService::comparisonCompleted, this, [this]() {
        refreshResultView();
    });
    connect(&m_compareSession, &mergeqt::app::BinaryCompareSessionService::comparisonFailed, this, [this](const QString &message) {
        m_summaryLabel->setText(message);
        m_statusLabel->setText(message);
        emit pageStatusChanged(message);
    });
    connect(&m_compareSession,
            &mergeqt::app::BinaryCompareSessionService::comparisonStatusChanged,
            this,
            [this](const QString &message) {
                m_statusLabel->setText(message);
                emit pageStatusChanged(message);
            });
}

void BinaryCompareTabPage::loadStartupState()
{
    if (!m_descriptor.leftPath.isEmpty() || !m_descriptor.rightPath.isEmpty()) {
        m_inputHeader->leftPathEdit()->setText(m_descriptor.leftPath);
        m_inputHeader->rightPathEdit()->setText(m_descriptor.rightPath);
    }

    if (!m_inputHeader->leftPathEdit()->text().isEmpty() && !m_inputHeader->rightPathEdit()->text().isEmpty())
        executeCompare();
}

void BinaryCompareTabPage::refreshResultView()
{
    const auto result = m_compareSession.currentResult();
    m_leftMetaLabel->setText(
        tr("Path: %1\nReadable: %2\nSize: %3")
            .arg(result.leftPath, result.leftReadable ? tr("Yes") : tr("No"), sizeText(result.leftSize)));
    m_rightMetaLabel->setText(
        tr("Path: %1\nReadable: %2\nSize: %3")
            .arg(result.rightPath, result.rightReadable ? tr("Yes") : tr("No"), sizeText(result.rightSize)));

    const QString summary = result.identicalContent ? tr("Binary files are byte-identical.")
                                                    : (result.sameSize ? tr("Binary files have matching sizes but different contents.")
                                                                       : tr("Binary files differ in size."));
    m_summaryLabel->setText(summary);

    const QString detail = tr("Compared bytes: %1\nSame size: %2\nExact match: %3")
                               .arg(result.comparedBytes)
                               .arg(result.sameSize ? tr("Yes") : tr("No"))
                               .arg(result.identicalContent ? tr("Yes") : tr("No"));
    m_analysisEdit->setPlainText(detail);
}

void BinaryCompareTabPage::emitPageState()
{
    emit pageTitleChanged(pageTitle());
    emit pageStatusChanged(pageStatusText());
}

QString BinaryCompareTabPage::fileLabelForPath(const QString &path) const
{
    const QFileInfo info(path);
    return info.fileName().isEmpty() ? tr("Binary File") : info.fileName();
}

QString BinaryCompareTabPage::sizeText(qint64 size) const
{
    return size < 0 ? tr("Unknown") : tr("%1 bytes").arg(size);
}

} // namespace mergeqt::ui

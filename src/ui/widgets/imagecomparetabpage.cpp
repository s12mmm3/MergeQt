#include "ui/widgets/imagecomparetabpage.h"

#include "ui/widgets/compareinputheader.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QSplitter>
#include <QVBoxLayout>

namespace mergeqt::ui {

namespace {

constexpr QSize kPreviewSize(420, 320);

} // namespace

ImageCompareTabPage::ImageCompareTabPage(const mergeqt::app::ComparePageDescriptor &descriptor, QWidget *parent)
    : CompareTabPage(parent)
    , m_compareSession(this)
    , m_descriptor(descriptor)
{
    buildUi();
    connectSignals();
    loadStartupState();
}

mergeqt::app::ComparePageType ImageCompareTabPage::pageType() const
{
    return mergeqt::app::ComparePageType::Image;
}

QString ImageCompareTabPage::pageTitle() const
{
    const QString left = fileLabelForPath(m_inputHeader ? m_inputHeader->leftPathEdit()->text().trimmed() : QString());
    const QString right = fileLabelForPath(m_inputHeader ? m_inputHeader->rightPathEdit()->text().trimmed() : QString());
    return tr("%1 vs %2").arg(left, right);
}

QString ImageCompareTabPage::pageStatusText() const
{
    return m_statusLabel ? m_statusLabel->text() : QString();
}

CompareTabCapabilities ImageCompareTabPage::capabilities() const
{
    return { true, true, true, false, true };
}

bool ImageCompareTabPage::chooseLeftFile()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Open left image"), m_inputHeader->leftPathEdit()->text());
    if (path.isEmpty())
        return false;

    m_inputHeader->leftPathEdit()->setText(path);
    return executeCompare();
}

bool ImageCompareTabPage::chooseRightFile()
{
    const QString path =
        QFileDialog::getOpenFileName(this, tr("Open right image"), m_inputHeader->rightPathEdit()->text());
    if (path.isEmpty())
        return false;

    m_inputHeader->rightPathEdit()->setText(path);
    return executeCompare();
}

bool ImageCompareTabPage::reloadInputs()
{
    const bool ok = m_compareSession.reloadCurrentFiles();
    if (ok)
        emitPageState();
    return ok;
}

bool ImageCompareTabPage::swapInputs()
{
    if (!m_compareSession.swapInputs())
        return false;

    const auto result = m_compareSession.currentResult();
    m_inputHeader->leftPathEdit()->setText(result.leftPath);
    m_inputHeader->rightPathEdit()->setText(result.rightPath);
    emitPageState();
    return true;
}

bool ImageCompareTabPage::executeCompare()
{
    if (!m_compareSession.compareFiles(m_inputHeader->leftPathEdit()->text().trimmed(),
                                       m_inputHeader->rightPathEdit()->text().trimmed()))
        return false;

    emitPageState();
    return true;
}

void ImageCompareTabPage::navigateDifferenceByOffset(int)
{
}

void ImageCompareTabPage::applyWindowSettings()
{
    updatePreviewLabels();
}

void ImageCompareTabPage::retranslateUi()
{
    if (m_pageSummaryLabel) {
        m_pageSummaryLabel->setText(
            tr("Use this page to compare two images by format, dimensions, and exact pixel equality. This is the image-specific shell that later overlay, difference heatmap, and zoom-sync features can build on."));
    }
    if (m_inputHeader)
        m_inputHeader->setLabelTexts(tr("Left image"), tr("Right image"));
    if (m_inputHeader)
        m_inputHeader->retranslateUi();
    if (m_summaryLabel && m_summaryLabel->text().isEmpty())
        m_summaryLabel->setText(tr("Choose two images before running image compare."));
    if (m_statusLabel && m_statusLabel->text().isEmpty())
        m_statusLabel->setText(m_summaryLabel ? m_summaryLabel->text() : tr("Choose two images before running image compare."));
    refreshResultView();
}

void ImageCompareTabPage::buildUi()
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

    auto *previewSplitter = new QSplitter(Qt::Horizontal, this);

    auto *leftPanel = new QWidget(previewSplitter);
    auto *leftPanelLayout = new QVBoxLayout(leftPanel);
    leftPanelLayout->setContentsMargins(0, 0, 0, 0);
    m_leftMetaLabel = new QLabel(leftPanel);
    m_leftMetaLabel->setWordWrap(true);
    m_leftPreviewLabel = new QLabel(leftPanel);
    m_leftPreviewLabel->setAlignment(Qt::AlignCenter);
    m_leftPreviewLabel->setMinimumSize(kPreviewSize);
    m_leftPreviewLabel->setFrameShape(QFrame::StyledPanel);
    leftPanelLayout->addWidget(m_leftMetaLabel);
    leftPanelLayout->addWidget(m_leftPreviewLabel, 1);

    auto *rightPanel = new QWidget(previewSplitter);
    auto *rightPanelLayout = new QVBoxLayout(rightPanel);
    rightPanelLayout->setContentsMargins(0, 0, 0, 0);
    m_rightMetaLabel = new QLabel(rightPanel);
    m_rightMetaLabel->setWordWrap(true);
    m_rightPreviewLabel = new QLabel(rightPanel);
    m_rightPreviewLabel->setAlignment(Qt::AlignCenter);
    m_rightPreviewLabel->setMinimumSize(kPreviewSize);
    m_rightPreviewLabel->setFrameShape(QFrame::StyledPanel);
    rightPanelLayout->addWidget(m_rightMetaLabel);
    rightPanelLayout->addWidget(m_rightPreviewLabel, 1);

    previewSplitter->setStretchFactor(0, 1);
    previewSplitter->setStretchFactor(1, 1);
    rootLayout->addWidget(previewSplitter, 1);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    rootLayout->addWidget(m_statusLabel);

    connect(m_inputHeader, &CompareInputHeader::leftBrowseRequested, this, &ImageCompareTabPage::chooseLeftFile);
    connect(m_inputHeader, &CompareInputHeader::rightBrowseRequested, this, &ImageCompareTabPage::chooseRightFile);
    connect(m_inputHeader, &CompareInputHeader::compareRequested, this, &ImageCompareTabPage::executeCompare);
    connect(m_inputHeader, &CompareInputHeader::reloadRequested, this, &ImageCompareTabPage::reloadInputs);
    connect(m_inputHeader, &CompareInputHeader::swapRequested, this, &ImageCompareTabPage::swapInputs);
    retranslateUi();
}

void ImageCompareTabPage::connectSignals()
{
    connect(&m_compareSession, &mergeqt::app::ImageCompareSessionService::comparisonCompleted, this, [this]() {
        refreshResultView();
    });
    connect(&m_compareSession, &mergeqt::app::ImageCompareSessionService::comparisonFailed, this, [this](const QString &message) {
        m_summaryLabel->setText(message);
        m_statusLabel->setText(message);
        emit pageStatusChanged(message);
    });
    connect(&m_compareSession,
            &mergeqt::app::ImageCompareSessionService::comparisonStatusChanged,
            this,
            [this](const QString &message) {
                m_statusLabel->setText(message);
                emit pageStatusChanged(message);
            });
}

void ImageCompareTabPage::loadStartupState()
{
    if (!m_descriptor.leftPath.isEmpty() || !m_descriptor.rightPath.isEmpty()) {
        m_inputHeader->leftPathEdit()->setText(m_descriptor.leftPath);
        m_inputHeader->rightPathEdit()->setText(m_descriptor.rightPath);
    }

    if (!m_inputHeader->leftPathEdit()->text().isEmpty() && !m_inputHeader->rightPathEdit()->text().isEmpty())
        executeCompare();
}

void ImageCompareTabPage::refreshResultView()
{
    const auto result = m_compareSession.currentResult();
    m_leftMetaLabel->setText(imageMetaText(result.leftPath, result.leftSize, result.leftFormat));
    m_rightMetaLabel->setText(imageMetaText(result.rightPath, result.rightSize, result.rightFormat));

    if (result.identicalPixels)
        m_summaryLabel->setText(tr("Images are pixel-identical."));
    else if (!result.sameDimensions)
        m_summaryLabel->setText(tr("Images loaded successfully, but their dimensions differ."));
    else
        m_summaryLabel->setText(tr("Images differ in %1 pixels.").arg(result.differingPixelCount));

    updatePreviewLabels();
}

void ImageCompareTabPage::updatePreviewLabels()
{
    const QImage leftImage = m_compareSession.currentLeftImage();
    const QImage rightImage = m_compareSession.currentRightImage();
    if (m_leftPreviewLabel) {
        if (leftImage.isNull())
            m_leftPreviewLabel->setText(tr("No left image loaded"));
        else
            m_leftPreviewLabel->setPixmap(QPixmap::fromImage(leftImage).scaled(kPreviewSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    if (m_rightPreviewLabel) {
        if (rightImage.isNull())
            m_rightPreviewLabel->setText(tr("No right image loaded"));
        else
            m_rightPreviewLabel->setPixmap(QPixmap::fromImage(rightImage).scaled(kPreviewSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void ImageCompareTabPage::emitPageState()
{
    emit pageTitleChanged(pageTitle());
    emit pageStatusChanged(pageStatusText());
}

QString ImageCompareTabPage::fileLabelForPath(const QString &path) const
{
    const QFileInfo info(path);
    return info.fileName().isEmpty() ? tr("Image") : info.fileName();
}

QString ImageCompareTabPage::imageMetaText(const QString &path, const QSize &size, const QString &format) const
{
    return tr("Path: %1\nFormat: %2\nDimensions: %3 x %4")
        .arg(path, format.isEmpty() ? tr("Unknown") : format)
        .arg(size.width())
        .arg(size.height());
}

} // namespace mergeqt::ui

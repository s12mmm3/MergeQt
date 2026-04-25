#include "app/compare/imagecomparesessionservice.h"

#include <QImageReader>

namespace mergeqt::app {

namespace {

qint64 countDifferingPixels(const QImage &left, const QImage &right)
{
    if (left.size() != right.size())
        return -1;

    const QImage leftRgba = left.convertToFormat(QImage::Format_RGBA8888);
    const QImage rightRgba = right.convertToFormat(QImage::Format_RGBA8888);
    qint64 differingPixels = 0;

    for (int y = 0; y < leftRgba.height(); ++y) {
        const auto *leftLine = reinterpret_cast<const QRgb *>(leftRgba.constScanLine(y));
        const auto *rightLine = reinterpret_cast<const QRgb *>(rightRgba.constScanLine(y));
        for (int x = 0; x < leftRgba.width(); ++x) {
            if (leftLine[x] != rightLine[x])
                ++differingPixels;
        }
    }

    return differingPixels;
}

} // namespace

ImageCompareSessionService::ImageCompareSessionService(QObject *parent)
    : QObject(parent)
{
}

mergeqt::core::ImageCompareResult ImageCompareSessionService::currentResult() const
{
    return m_currentResult;
}

QImage ImageCompareSessionService::currentLeftImage() const
{
    return m_leftImage;
}

QImage ImageCompareSessionService::currentRightImage() const
{
    return m_rightImage;
}

QString ImageCompareSessionService::currentStatusText() const
{
    return m_statusText;
}

bool ImageCompareSessionService::compareFiles(const QString &leftPath, const QString &rightPath)
{
    m_currentResult = {};
    m_currentResult.leftPath = leftPath;
    m_currentResult.rightPath = rightPath;
    return compareCurrentFiles();
}

bool ImageCompareSessionService::reloadCurrentFiles()
{
    if (m_currentResult.leftPath.isEmpty() || m_currentResult.rightPath.isEmpty()) {
        m_statusText = tr("Choose two image files before reloading the image compare.");
        emit comparisonFailed(m_statusText);
        emit comparisonStatusChanged(m_statusText);
        return false;
    }

    return compareCurrentFiles();
}

bool ImageCompareSessionService::swapInputs()
{
    if (m_currentResult.leftPath.isEmpty() && m_currentResult.rightPath.isEmpty()) {
        m_statusText = tr("Nothing to swap in the current image compare.");
        emit comparisonFailed(m_statusText);
        emit comparisonStatusChanged(m_statusText);
        return false;
    }

    qSwap(m_currentResult.leftPath, m_currentResult.rightPath);
    return compareCurrentFiles();
}

bool ImageCompareSessionService::compareCurrentFiles()
{
    m_leftImage = {};
    m_rightImage = {};

    if (m_currentResult.leftPath.isEmpty() || m_currentResult.rightPath.isEmpty()) {
        m_statusText = tr("Choose two image files before running image compare.");
        emit comparisonFailed(m_statusText);
        emit comparisonStatusChanged(m_statusText);
        return false;
    }

    QImageReader leftReader(m_currentResult.leftPath);
    QImageReader rightReader(m_currentResult.rightPath);
    m_currentResult.leftFormat = QString::fromLatin1(leftReader.format());
    m_currentResult.rightFormat = QString::fromLatin1(rightReader.format());
    m_leftImage = leftReader.read();
    m_rightImage = rightReader.read();
    m_currentResult.leftLoaded = !m_leftImage.isNull();
    m_currentResult.rightLoaded = !m_rightImage.isNull();

    if (!m_currentResult.leftLoaded || !m_currentResult.rightLoaded) {
        m_statusText = tr("Image compare needs two readable image files.");
        emit comparisonFailed(m_statusText);
        emit comparisonStatusChanged(m_statusText);
        return false;
    }

    m_currentResult.leftSize = m_leftImage.size();
    m_currentResult.rightSize = m_rightImage.size();
    m_currentResult.sameDimensions = m_currentResult.leftSize == m_currentResult.rightSize;
    m_currentResult.differingPixelCount = countDifferingPixels(m_leftImage, m_rightImage);
    m_currentResult.identicalPixels = m_currentResult.differingPixelCount == 0;

    if (m_currentResult.identicalPixels)
        m_statusText = tr("Images are identical.");
    else if (!m_currentResult.sameDimensions)
        m_statusText = tr("Images differ: dimensions do not match.");
    else
        m_statusText = tr("Images differ in %1 pixels.").arg(m_currentResult.differingPixelCount);

    emit comparisonCompleted();
    emit comparisonStatusChanged(m_statusText);
    return true;
}

} // namespace mergeqt::app

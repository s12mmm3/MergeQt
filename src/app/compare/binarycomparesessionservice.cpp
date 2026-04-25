#include "app/compare/binarycomparesessionservice.h"

#include <QFile>
#include <QFileInfo>

namespace mergeqt::app {

namespace {

constexpr qint64 kBinaryCompareChunkSize = 64 * 1024;

} // namespace

BinaryCompareSessionService::BinaryCompareSessionService(QObject *parent)
    : QObject(parent)
{
}

mergeqt::core::BinaryCompareResult BinaryCompareSessionService::currentResult() const
{
    return m_currentResult;
}

QString BinaryCompareSessionService::currentStatusText() const
{
    return m_statusText;
}

bool BinaryCompareSessionService::compareFiles(const QString &leftPath, const QString &rightPath)
{
    m_currentResult = {};
    m_currentResult.leftPath = leftPath;
    m_currentResult.rightPath = rightPath;
    return compareCurrentFiles();
}

bool BinaryCompareSessionService::reloadCurrentFiles()
{
    if (m_currentResult.leftPath.isEmpty() || m_currentResult.rightPath.isEmpty()) {
        m_statusText = tr("Choose two files before reloading the binary compare.");
        emit comparisonFailed(m_statusText);
        emit comparisonStatusChanged(m_statusText);
        return false;
    }

    return compareCurrentFiles();
}

bool BinaryCompareSessionService::swapInputs()
{
    if (m_currentResult.leftPath.isEmpty() && m_currentResult.rightPath.isEmpty()) {
        m_statusText = tr("Nothing to swap in the current binary compare.");
        emit comparisonFailed(m_statusText);
        emit comparisonStatusChanged(m_statusText);
        return false;
    }

    qSwap(m_currentResult.leftPath, m_currentResult.rightPath);
    return compareCurrentFiles();
}

bool BinaryCompareSessionService::compareCurrentFiles()
{
    if (m_currentResult.leftPath.isEmpty() || m_currentResult.rightPath.isEmpty()) {
        m_statusText = tr("Choose two files before running binary compare.");
        emit comparisonFailed(m_statusText);
        emit comparisonStatusChanged(m_statusText);
        return false;
    }

    const QFileInfo leftInfo(m_currentResult.leftPath);
    const QFileInfo rightInfo(m_currentResult.rightPath);
    m_currentResult.leftExists = leftInfo.exists();
    m_currentResult.rightExists = rightInfo.exists();
    m_currentResult.leftReadable = leftInfo.isFile() && leftInfo.isReadable();
    m_currentResult.rightReadable = rightInfo.isFile() && rightInfo.isReadable();
    m_currentResult.leftSize = leftInfo.isFile() ? leftInfo.size() : -1;
    m_currentResult.rightSize = rightInfo.isFile() ? rightInfo.size() : -1;
    m_currentResult.sameSize = m_currentResult.leftSize >= 0
                               && m_currentResult.leftSize == m_currentResult.rightSize;
    m_currentResult.identicalContent = false;
    m_currentResult.comparedBytes = 0;

    if (!m_currentResult.leftReadable || !m_currentResult.rightReadable) {
        m_statusText = tr("Binary compare needs two readable files.");
        emit comparisonFailed(m_statusText);
        emit comparisonStatusChanged(m_statusText);
        return false;
    }

    if (!m_currentResult.sameSize) {
        m_statusText = tr("Binary files differ: file sizes do not match.");
        emit comparisonCompleted();
        emit comparisonStatusChanged(m_statusText);
        return true;
    }

    QFile leftFile(m_currentResult.leftPath);
    QFile rightFile(m_currentResult.rightPath);
    if (!leftFile.open(QIODevice::ReadOnly) || !rightFile.open(QIODevice::ReadOnly)) {
        m_statusText = tr("Failed to open one or both binary files.");
        emit comparisonFailed(m_statusText);
        emit comparisonStatusChanged(m_statusText);
        return false;
    }

    bool identical = true;
    while (!leftFile.atEnd() && !rightFile.atEnd()) {
        const QByteArray leftChunk = leftFile.read(kBinaryCompareChunkSize);
        const QByteArray rightChunk = rightFile.read(kBinaryCompareChunkSize);
        m_currentResult.comparedBytes += qMin(leftChunk.size(), rightChunk.size());
        if (leftChunk != rightChunk) {
            identical = false;
            break;
        }
    }

    m_currentResult.identicalContent = identical;
    m_statusText = identical ? tr("Binary files are identical.") : tr("Binary files differ.");
    emit comparisonCompleted();
    emit comparisonStatusChanged(m_statusText);
    return true;
}

} // namespace mergeqt::app

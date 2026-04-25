#include "app/compare/comparesessionservice.h"

#include "app/logging/logger.h"
#include "app/settings/settingsservice.h"

namespace mergeqt::app {

mergeqt::core::TextCompareResult CompareSessionService::currentResult() const
{
    return m_currentResult;
}

mergeqt::core::TextCompareInput CompareSessionService::currentInput() const
{
    return m_currentInput;
}

mergeqt::core::TextDocument CompareSessionService::currentLeftDocument() const
{
    return m_currentLeftDocument;
}

mergeqt::core::TextDocument CompareSessionService::currentRightDocument() const
{
    return m_currentRightDocument;
}

bool CompareSessionService::compareText(const QString &leftText,
                                        const QString &rightText,
                                        const QString &leftLabel,
                                        const QString &rightLabel)
{
    const auto *settings = SettingsService::instance()->compare();
    m_currentInput.leftText = leftText;
    m_currentInput.rightText = rightText;
    m_currentInput.leftLabel = leftLabel;
    m_currentInput.rightLabel = rightLabel;
    m_currentInput.ignoreCase = settings->ignoreCase();
    m_currentInput.trimWhitespace = settings->trimWhitespace();

    m_currentLeftDocument = m_documentLoader.fromText(leftText, leftLabel);
    m_currentRightDocument = m_documentLoader.fromText(rightText, rightLabel);
    return compareCurrentDocuments();
}

bool CompareSessionService::comparePathsOrText(const QString &leftPath,
                                               const QString &rightPath,
                                               const QString &leftFallbackText,
                                               const QString &rightFallbackText,
                                               const QString &leftLabel,
                                               const QString &rightLabel)
{
    QString errorMessage;

    if (!leftPath.trimmed().isEmpty()) {
        if (!m_documentLoader.fromFile(leftPath, m_currentLeftDocument, errorMessage)) {
            emit comparisonFailed(errorMessage);
            emit comparisonStatusChanged(errorMessage);
            return false;
        }
    } else {
        m_currentLeftDocument = m_documentLoader.fromText(leftFallbackText, leftLabel);
    }

    if (!rightPath.trimmed().isEmpty()) {
        if (!m_documentLoader.fromFile(rightPath, m_currentRightDocument, errorMessage)) {
            emit comparisonFailed(errorMessage);
            emit comparisonStatusChanged(errorMessage);
            return false;
        }
    } else {
        m_currentRightDocument = m_documentLoader.fromText(rightFallbackText, rightLabel);
    }

    auto *runtime = SettingsService::instance()->runtime();
    if (!leftPath.trimmed().isEmpty())
        runtime->set_lastLeftPath(leftPath);
    if (!rightPath.trimmed().isEmpty())
        runtime->set_lastRightPath(rightPath);

    const auto *settings = SettingsService::instance()->compare();
    m_currentInput.leftText = m_currentLeftDocument.text;
    m_currentInput.rightText = m_currentRightDocument.text;
    m_currentInput.leftLabel = m_currentLeftDocument.label;
    m_currentInput.rightLabel = m_currentRightDocument.label;
    m_currentInput.ignoreCase = settings->ignoreCase();
    m_currentInput.trimWhitespace = settings->trimWhitespace();
    return compareCurrentDocuments();
}

bool CompareSessionService::compareFiles(const QString &leftPath, const QString &rightPath)
{
    return comparePathsOrText(leftPath, rightPath);
}

bool CompareSessionService::reloadCurrentFiles()
{
    if (m_currentLeftDocument.sourcePath.isEmpty() && m_currentRightDocument.sourcePath.isEmpty()) {
        const QString message = tr("Reload is only available when at least one side comes from a file.");
        emit comparisonFailed(message);
        emit comparisonStatusChanged(message);
        return false;
    }

    return comparePathsOrText(m_currentLeftDocument.sourcePath,
                              m_currentRightDocument.sourcePath,
                              QString(),
                              QString(),
                              m_currentLeftDocument.label,
                              m_currentRightDocument.label);
}

bool CompareSessionService::swapInputs()
{
    qSwap(m_currentInput.leftText, m_currentInput.rightText);
    qSwap(m_currentInput.leftLabel, m_currentInput.rightLabel);
    qSwap(m_currentLeftDocument, m_currentRightDocument);
    return compareCurrentDocuments();
}

CompareSessionService::CompareSessionService(QObject *parent)
    : QObject(parent)
{
}

bool CompareSessionService::compareCurrentDocuments()
{
    const auto normalizedLeft = m_normalizer.normalize(m_currentLeftDocument, m_currentInput.ignoreCase, m_currentInput.trimWhitespace);
    const auto normalizedRight = m_normalizer.normalize(m_currentRightDocument, m_currentInput.ignoreCase, m_currentInput.trimWhitespace);

    m_currentResult = m_engine.compare(normalizedLeft, normalizedRight);

    int differenceBlocks = 0;
    for (const auto &block : m_currentResult.blocks) {
        if (block.isDifference())
            ++differenceBlocks;
    }

    const QString message = tr("Compared %1 difference rows in %2 blocks (%3 equal, %4 inserted, %5 deleted, %6 replaced).")
                                .arg(m_currentResult.stats.differenceCount())
                                .arg(differenceBlocks)
                                .arg(m_currentResult.stats.equalCount)
                                .arg(m_currentResult.stats.insertCount)
                                .arg(m_currentResult.stats.deleteCount)
                                .arg(m_currentResult.stats.replaceCount);
    LOG_INFO << message;
    emit comparisonStatusChanged(message);
    emit comparisonCompleted();
    return true;
}

} // namespace mergeqt::app

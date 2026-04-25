#pragma once

#include "core/text/lineardiffengine.h"
#include "core/text/textdocumentloader.h"
#include "core/text/textnormalizer.h"

#include <QObject>

namespace mergeqt::app {

class CompareSessionService : public QObject
{
    Q_OBJECT

public:
    explicit CompareSessionService(QObject *parent = nullptr);

    [[nodiscard]] mergeqt::core::TextCompareResult currentResult() const;
    [[nodiscard]] mergeqt::core::TextCompareInput currentInput() const;
    [[nodiscard]] mergeqt::core::TextDocument currentLeftDocument() const;
    [[nodiscard]] mergeqt::core::TextDocument currentRightDocument() const;

    Q_INVOKABLE bool compareText(const QString &leftText,
                                 const QString &rightText,
                                 const QString &leftLabel = {},
                                 const QString &rightLabel = {});
    Q_INVOKABLE bool comparePathsOrText(const QString &leftPath,
                                        const QString &rightPath,
                                        const QString &leftFallbackText = {},
                                        const QString &rightFallbackText = {},
                                        const QString &leftLabel = {},
                                        const QString &rightLabel = {});
    Q_INVOKABLE bool compareFiles(const QString &leftPath, const QString &rightPath);
    Q_INVOKABLE bool reloadCurrentFiles();
    Q_INVOKABLE bool swapInputs();

Q_SIGNALS:
    void comparisonCompleted();
    void comparisonFailed(const QString &message);
    void comparisonStatusChanged(const QString &message);

private:
    bool compareCurrentDocuments();

    mergeqt::core::TextDocumentLoader m_documentLoader;
    mergeqt::core::TextNormalizer m_normalizer;
    mergeqt::core::MinimalLineDiffEngine m_engine;
    mergeqt::core::TextCompareInput m_currentInput;
    mergeqt::core::TextCompareResult m_currentResult;
    mergeqt::core::TextDocument m_currentLeftDocument;
    mergeqt::core::TextDocument m_currentRightDocument;
};

} // namespace mergeqt::app

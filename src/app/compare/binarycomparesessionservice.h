#pragma once

#include "core/binary/binarycomparetypes.h"

#include <QObject>

namespace mergeqt::app {

class BinaryCompareSessionService : public QObject
{
    Q_OBJECT

public:
    explicit BinaryCompareSessionService(QObject *parent = nullptr);

    [[nodiscard]] mergeqt::core::BinaryCompareResult currentResult() const;
    [[nodiscard]] QString currentStatusText() const;

    Q_INVOKABLE bool compareFiles(const QString &leftPath, const QString &rightPath);
    Q_INVOKABLE bool reloadCurrentFiles();
    Q_INVOKABLE bool swapInputs();

Q_SIGNALS:
    void comparisonCompleted();
    void comparisonFailed(const QString &message);
    void comparisonStatusChanged(const QString &message);

private:
    bool compareCurrentFiles();

    mergeqt::core::BinaryCompareResult m_currentResult;
    QString m_statusText;
};

} // namespace mergeqt::app

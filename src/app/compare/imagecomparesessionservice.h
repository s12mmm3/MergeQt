#pragma once

#include "core/image/imagecomparetypes.h"

#include <QImage>
#include <QObject>

namespace mergeqt::app {

class ImageCompareSessionService : public QObject
{
    Q_OBJECT

public:
    explicit ImageCompareSessionService(QObject *parent = nullptr);

    [[nodiscard]] mergeqt::core::ImageCompareResult currentResult() const;
    [[nodiscard]] QImage currentLeftImage() const;
    [[nodiscard]] QImage currentRightImage() const;
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

    mergeqt::core::ImageCompareResult m_currentResult;
    QImage m_leftImage;
    QImage m_rightImage;
    QString m_statusText;
};

} // namespace mergeqt::app

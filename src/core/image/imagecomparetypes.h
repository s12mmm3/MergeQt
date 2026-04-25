#pragma once

#include <QSize>
#include <QString>

namespace mergeqt::core {

struct ImageCompareResult
{
    QString leftPath;
    QString rightPath;
    bool leftLoaded = false;
    bool rightLoaded = false;
    QSize leftSize;
    QSize rightSize;
    QString leftFormat;
    QString rightFormat;
    bool sameDimensions = false;
    bool identicalPixels = false;
    qint64 differingPixelCount = -1;
};

} // namespace mergeqt::core

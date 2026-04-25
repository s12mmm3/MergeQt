#pragma once

#include <QString>

namespace mergeqt::core {

struct BinaryCompareResult
{
    QString leftPath;
    QString rightPath;
    bool leftExists = false;
    bool rightExists = false;
    bool leftReadable = false;
    bool rightReadable = false;
    qint64 leftSize = -1;
    qint64 rightSize = -1;
    bool sameSize = false;
    bool identicalContent = false;
    qint64 comparedBytes = 0;
};

} // namespace mergeqt::core

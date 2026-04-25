#pragma once

#include "core/folder/foldercomparetypes.h"

#include <QMap>
#include <QString>

namespace mergeqt::core {

class FolderCompareEngine
{
public:
    [[nodiscard]] bool compare(const QString &leftRootPath,
                               const QString &rightRootPath,
                               FolderCompareResult &result,
                               QString &errorMessage) const;

private:
    [[nodiscard]] bool scanDirectory(const QString &rootPath,
                                     QMap<QString, FolderScanEntry> &entries,
                                     QString &errorMessage) const;
    [[nodiscard]] FolderCompareEntry buildEntry(const QString &relativePath,
                                                const FolderScanEntry *leftEntry,
                                                const FolderScanEntry *rightEntry) const;
    [[nodiscard]] bool filesMatch(const QString &leftPath, const QString &rightPath) const;
};

} // namespace mergeqt::core

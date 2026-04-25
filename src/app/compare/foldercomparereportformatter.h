#pragma once

#include "core/folder/foldercomparetypes.h"

#include <QString>

namespace mergeqt::app {

enum class FolderCompareReportFormat
{
    Text,
    Html
};

class FolderCompareReportFormatter
{
public:
    [[nodiscard]] static QString defaultSuffix(FolderCompareReportFormat format);
    [[nodiscard]] static QString formatReport(const mergeqt::core::FolderCompareResult &result,
                                              const QVector<mergeqt::core::FolderCompareEntry> &entries,
                                              FolderCompareReportFormat format);

private:
    [[nodiscard]] static QString statusText(mergeqt::core::FolderDiffStatus status);
    [[nodiscard]] static QString entryKindText(mergeqt::core::FolderEntryKind kind);
};

} // namespace mergeqt::app

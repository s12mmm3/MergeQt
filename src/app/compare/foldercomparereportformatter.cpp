#include "app/compare/foldercomparereportformatter.h"

#include <QDateTime>
#include <QTextStream>

namespace mergeqt::app {

QString FolderCompareReportFormatter::defaultSuffix(FolderCompareReportFormat format)
{
    switch (format) {
    case FolderCompareReportFormat::Text:
        return QStringLiteral("txt");
    case FolderCompareReportFormat::Html:
        return QStringLiteral("html");
    }

    return QStringLiteral("txt");
}

QString FolderCompareReportFormatter::formatReport(const mergeqt::core::FolderCompareResult &result,
                                                   const QVector<mergeqt::core::FolderCompareEntry> &entries,
                                                   FolderCompareReportFormat format)
{
    QString output;
    QTextStream stream(&output);
    const QString generatedAt = QDateTime::currentDateTime().toString(Qt::ISODate);

    if (format == FolderCompareReportFormat::Html) {
        stream << "<html><head><meta charset=\"utf-8\"><title>MergeQt Folder Compare Report</title>"
               << "<style>body{font-family:sans-serif;}table{border-collapse:collapse;width:100%;}"
               << "th,td{border:1px solid #c7ccd1;padding:6px 8px;text-align:left;}"
               << "th{background:#eef2f6;}tr:nth-child(even){background:#f8fafc;}</style></head><body>";
        stream << "<h1>MergeQt Folder Compare Report</h1>";
        stream << "<p>Generated: " << generatedAt.toHtmlEscaped() << "</p>";
        stream << "<p>Left: " << result.leftRootPath.toHtmlEscaped() << "<br/>Right: "
               << result.rightRootPath.toHtmlEscaped() << "</p>";
        stream << "<p>Total entries: " << result.entries.size() << ", visible entries: " << entries.size()
               << ", differences: " << result.stats.differenceCount() << "</p>";
        stream << "<table><thead><tr><th>Status</th><th>Relative Path</th><th>Left Type</th><th>Right Type</th><th>Left Size</th><th>Right Size</th></tr></thead><tbody>";
        for (const auto &entry : entries) {
            stream << "<tr><td>" << statusText(entry.status).toHtmlEscaped() << "</td><td>"
                   << entry.relativePath.toHtmlEscaped() << "</td><td>" << entryKindText(entry.leftKind).toHtmlEscaped()
                   << "</td><td>" << entryKindText(entry.rightKind).toHtmlEscaped() << "</td><td>" << entry.leftSize
                   << "</td><td>" << entry.rightSize << "</td></tr>";
        }
        stream << "</tbody></table></body></html>";
        return output;
    }

    stream << "MergeQt Folder Compare Report\n";
    stream << "Generated: " << generatedAt << "\n";
    stream << "Left: " << result.leftRootPath << "\n";
    stream << "Right: " << result.rightRootPath << "\n";
    stream << "Total entries: " << result.entries.size() << "\n";
    stream << "Visible entries: " << entries.size() << "\n";
    stream << "Differences: " << result.stats.differenceCount() << "\n\n";
    stream << "Status\tRelative Path\tLeft Type\tRight Type\tLeft Size\tRight Size\n";
    for (const auto &entry : entries) {
        stream << statusText(entry.status) << '\t' << entry.relativePath << '\t' << entryKindText(entry.leftKind) << '\t'
               << entryKindText(entry.rightKind) << '\t' << entry.leftSize << '\t' << entry.rightSize << '\n';
    }
    return output;
}

QString FolderCompareReportFormatter::statusText(mergeqt::core::FolderDiffStatus status)
{
    switch (status) {
    case mergeqt::core::FolderDiffStatus::Identical:
        return QStringLiteral("Identical");
    case mergeqt::core::FolderDiffStatus::LeftOnly:
        return QStringLiteral("Left Only");
    case mergeqt::core::FolderDiffStatus::RightOnly:
        return QStringLiteral("Right Only");
    case mergeqt::core::FolderDiffStatus::ContentMismatch:
        return QStringLiteral("Different Content");
    case mergeqt::core::FolderDiffStatus::TypeMismatch:
        return QStringLiteral("Type Mismatch");
    }

    return QStringLiteral("Unknown");
}

QString FolderCompareReportFormatter::entryKindText(mergeqt::core::FolderEntryKind kind)
{
    switch (kind) {
    case mergeqt::core::FolderEntryKind::Missing:
        return QStringLiteral("Missing");
    case mergeqt::core::FolderEntryKind::File:
        return QStringLiteral("File");
    case mergeqt::core::FolderEntryKind::Directory:
        return QStringLiteral("Folder");
    case mergeqt::core::FolderEntryKind::Other:
        return QStringLiteral("Other");
    }

    return QStringLiteral("Other");
}

} // namespace mergeqt::app

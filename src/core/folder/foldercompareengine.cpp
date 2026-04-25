#include "core/folder/foldercompareengine.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QSet>

namespace {

using namespace mergeqt::core;

FolderEntryKind entryKindFromFileInfo(const QFileInfo &info)
{
    if (info.isDir())
        return FolderEntryKind::Directory;
    if (info.isFile())
        return FolderEntryKind::File;
    return FolderEntryKind::Other;
}

} // namespace

namespace mergeqt::core {

bool FolderCompareEngine::compare(const QString &leftRootPath,
                                  const QString &rightRootPath,
                                  FolderCompareResult &result,
                                  QString &errorMessage) const
{
    result = {};
    result.leftRootPath = QDir::cleanPath(leftRootPath);
    result.rightRootPath = QDir::cleanPath(rightRootPath);

    QMap<QString, FolderScanEntry> leftEntries;
    QMap<QString, FolderScanEntry> rightEntries;

    if (!scanDirectory(result.leftRootPath, leftEntries, errorMessage))
        return false;
    if (!scanDirectory(result.rightRootPath, rightEntries, errorMessage))
        return false;

    QSet<QString> allPaths = QSet<QString>(leftEntries.keyBegin(), leftEntries.keyEnd());
    allPaths.unite(QSet<QString>(rightEntries.keyBegin(), rightEntries.keyEnd()));

    QStringList sortedPaths = allPaths.values();
    std::sort(sortedPaths.begin(), sortedPaths.end());

    for (const QString &relativePath : sortedPaths) {
        const auto leftIt = leftEntries.constFind(relativePath);
        const auto rightIt = rightEntries.constFind(relativePath);
        const FolderScanEntry *leftEntry = leftIt == leftEntries.cend() ? nullptr : &leftIt.value();
        const FolderScanEntry *rightEntry = rightIt == rightEntries.cend() ? nullptr : &rightIt.value();

        FolderCompareEntry entry = buildEntry(relativePath, leftEntry, rightEntry);
        if (entry.leftKind == FolderEntryKind::File
            && entry.rightKind == FolderEntryKind::File
            && entry.status == FolderDiffStatus::Identical
            && !filesMatch(leftEntry->absolutePath, rightEntry->absolutePath)) {
            entry.status = FolderDiffStatus::ContentMismatch;
        }

        switch (entry.status) {
        case FolderDiffStatus::Identical:
            ++result.stats.identicalCount;
            break;
        case FolderDiffStatus::LeftOnly:
            ++result.stats.leftOnlyCount;
            break;
        case FolderDiffStatus::RightOnly:
            ++result.stats.rightOnlyCount;
            break;
        case FolderDiffStatus::ContentMismatch:
            ++result.stats.contentMismatchCount;
            break;
        case FolderDiffStatus::TypeMismatch:
            ++result.stats.typeMismatchCount;
            break;
        }

        result.entries.append(entry);
    }

    return true;
}

bool FolderCompareEngine::scanDirectory(const QString &rootPath,
                                        QMap<QString, FolderScanEntry> &entries,
                                        QString &errorMessage) const
{
    const QFileInfo rootInfo(rootPath);
    if (!rootInfo.exists() || !rootInfo.isDir()) {
        errorMessage = QObject::tr("Folder does not exist: %1").arg(rootPath);
        return false;
    }

    QDir rootDir(rootInfo.absoluteFilePath());
    QDirIterator it(rootDir.absolutePath(),
                    QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        const QFileInfo info = it.fileInfo();
        FolderScanEntry entry;
        entry.absolutePath = info.absoluteFilePath();
        entry.relativePath = QDir::fromNativeSeparators(rootDir.relativeFilePath(info.absoluteFilePath()));
        entry.kind = entryKindFromFileInfo(info);
        entry.size = info.isFile() ? info.size() : -1;
        entries.insert(entry.relativePath, entry);
    }

    return true;
}

FolderCompareEntry FolderCompareEngine::buildEntry(const QString &relativePath,
                                                   const FolderScanEntry *leftEntry,
                                                   const FolderScanEntry *rightEntry) const
{
    FolderCompareEntry entry;
    entry.relativePath = relativePath;

    if (leftEntry) {
        entry.leftAbsolutePath = leftEntry->absolutePath;
        entry.leftKind = leftEntry->kind;
        entry.leftSize = leftEntry->size;
    }

    if (rightEntry) {
        entry.rightAbsolutePath = rightEntry->absolutePath;
        entry.rightKind = rightEntry->kind;
        entry.rightSize = rightEntry->size;
    }

    if (!leftEntry)
        entry.status = FolderDiffStatus::RightOnly;
    else if (!rightEntry)
        entry.status = FolderDiffStatus::LeftOnly;
    else if (leftEntry->kind != rightEntry->kind)
        entry.status = FolderDiffStatus::TypeMismatch;
    else if (leftEntry->kind == FolderEntryKind::File && leftEntry->size != rightEntry->size)
        entry.status = FolderDiffStatus::ContentMismatch;
    else
        entry.status = FolderDiffStatus::Identical;

    return entry;
}

bool FolderCompareEngine::filesMatch(const QString &leftPath, const QString &rightPath) const
{
    QFile leftFile(leftPath);
    QFile rightFile(rightPath);
    if (!leftFile.open(QIODevice::ReadOnly) || !rightFile.open(QIODevice::ReadOnly))
        return false;

    constexpr qint64 kChunkSize = 64 * 1024;
    while (!leftFile.atEnd() || !rightFile.atEnd()) {
        if (leftFile.read(kChunkSize) != rightFile.read(kChunkSize))
            return false;
    }

    return true;
}

} // namespace mergeqt::core

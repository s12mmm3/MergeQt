#pragma once

#include <QDateTime>
#include <QMetaType>
#include <QString>
#include <QVector>

namespace mergeqt::app {

enum class ComparePageType
{
    Text,
    Folder,
    Binary,
    Image
};

struct ComparePageDescriptor
{
    ComparePageType type = ComparePageType::Text;
    QString title;
    QString leftPath;
    QString rightPath;
};

struct RecentCompareEntry
{
    ComparePageDescriptor descriptor;
    QDateTime accessedAt;
};

[[nodiscard]] QString compareDescriptorStorageKey(const ComparePageDescriptor &descriptor);
[[nodiscard]] QString serializeRecentCompareEntry(const RecentCompareEntry &entry);
[[nodiscard]] bool deserializeRecentCompareEntry(const QString &serialized, RecentCompareEntry &entry);
[[nodiscard]] QVector<RecentCompareEntry> upsertRecentCompareEntry(const QVector<RecentCompareEntry> &entries,
                                                                  const RecentCompareEntry &entry,
                                                                  int maxEntries = 12);
[[nodiscard]] QVector<RecentCompareEntry> removeRecentCompareEntry(const QVector<RecentCompareEntry> &entries,
                                                                   const ComparePageDescriptor &descriptor);

} // namespace mergeqt::app

Q_DECLARE_METATYPE(mergeqt::app::ComparePageDescriptor)
Q_DECLARE_METATYPE(mergeqt::app::RecentCompareEntry)

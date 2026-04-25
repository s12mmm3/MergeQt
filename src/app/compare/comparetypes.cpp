#include "app/compare/comparetypes.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace mergeqt::app {

namespace {

QString comparePageTypeKey(ComparePageType type)
{
    switch (type) {
    case ComparePageType::Text:
        return QStringLiteral("text");
    case ComparePageType::Folder:
        return QStringLiteral("folder");
    case ComparePageType::Binary:
        return QStringLiteral("binary");
    case ComparePageType::Image:
        return QStringLiteral("image");
    }

    return QStringLiteral("text");
}

ComparePageType comparePageTypeFromKey(const QString &typeKey)
{
    if (typeKey == QStringLiteral("folder"))
        return ComparePageType::Folder;
    if (typeKey == QStringLiteral("binary"))
        return ComparePageType::Binary;
    if (typeKey == QStringLiteral("image"))
        return ComparePageType::Image;
    return ComparePageType::Text;
}

} // namespace

QString compareDescriptorStorageKey(const ComparePageDescriptor &descriptor)
{
    return comparePageTypeKey(descriptor.type) + QLatin1Char('|') + descriptor.leftPath + QLatin1Char('|') + descriptor.rightPath;
}

QString serializeRecentCompareEntry(const RecentCompareEntry &entry)
{
    QJsonObject object;
    object.insert(QStringLiteral("type"), comparePageTypeKey(entry.descriptor.type));
    object.insert(QStringLiteral("title"), entry.descriptor.title);
    object.insert(QStringLiteral("leftPath"), entry.descriptor.leftPath);
    object.insert(QStringLiteral("rightPath"), entry.descriptor.rightPath);
    object.insert(QStringLiteral("accessedAt"), entry.accessedAt.toString(Qt::ISODateWithMs));
    return QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact));
}

bool deserializeRecentCompareEntry(const QString &serialized, RecentCompareEntry &entry)
{
    const auto document = QJsonDocument::fromJson(serialized.toUtf8());
    if (!document.isObject())
        return false;

    const auto object = document.object();
    entry.descriptor.type = comparePageTypeFromKey(object.value(QStringLiteral("type")).toString());
    entry.descriptor.title = object.value(QStringLiteral("title")).toString();
    entry.descriptor.leftPath = object.value(QStringLiteral("leftPath")).toString();
    entry.descriptor.rightPath = object.value(QStringLiteral("rightPath")).toString();
    entry.accessedAt = QDateTime::fromString(object.value(QStringLiteral("accessedAt")).toString(), Qt::ISODateWithMs);

    return !compareDescriptorStorageKey(entry.descriptor).isEmpty()
           && (!entry.descriptor.leftPath.isEmpty() || !entry.descriptor.rightPath.isEmpty());
}

QVector<RecentCompareEntry> upsertRecentCompareEntry(const QVector<RecentCompareEntry> &entries,
                                                     const RecentCompareEntry &entry,
                                                     int maxEntries)
{
    QVector<RecentCompareEntry> updatedEntries;
    updatedEntries.reserve(entries.size() + 1);

    const QString targetKey = compareDescriptorStorageKey(entry.descriptor);
    updatedEntries.append(entry);

    for (const auto &current : entries) {
        if (compareDescriptorStorageKey(current.descriptor) == targetKey)
            continue;
        updatedEntries.append(current);
        if (updatedEntries.size() >= maxEntries)
            break;
    }

    return updatedEntries;
}

QVector<RecentCompareEntry> removeRecentCompareEntry(const QVector<RecentCompareEntry> &entries,
                                                     const ComparePageDescriptor &descriptor)
{
    QVector<RecentCompareEntry> updatedEntries;
    updatedEntries.reserve(entries.size());

    const QString targetKey = compareDescriptorStorageKey(descriptor);
    for (const auto &current : entries) {
        if (compareDescriptorStorageKey(current.descriptor) == targetKey)
            continue;
        updatedEntries.append(current);
    }

    return updatedEntries;
}

} // namespace mergeqt::app

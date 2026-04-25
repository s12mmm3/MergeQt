#include "app/compare/comparetypes.h"

#include <QtTest>

using namespace mergeqt::app;

class RecentCompareTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void serializesAndDeserializesEntry()
    {
        RecentCompareEntry entry;
        entry.descriptor.type = ComparePageType::Folder;
        entry.descriptor.title = QStringLiteral("Folder Compare");
        entry.descriptor.leftPath = QStringLiteral("/tmp/left");
        entry.descriptor.rightPath = QStringLiteral("/tmp/right");
        entry.accessedAt = QDateTime::fromString(QStringLiteral("2026-04-17T10:20:30.123Z"), Qt::ISODateWithMs);

        RecentCompareEntry decoded;
        QVERIFY(deserializeRecentCompareEntry(serializeRecentCompareEntry(entry), decoded));
        QCOMPARE(decoded.descriptor.type, entry.descriptor.type);
        QCOMPARE(decoded.descriptor.title, entry.descriptor.title);
        QCOMPARE(decoded.descriptor.leftPath, entry.descriptor.leftPath);
        QCOMPARE(decoded.descriptor.rightPath, entry.descriptor.rightPath);
        QCOMPARE(decoded.accessedAt, entry.accessedAt);
    }

    void upsertsAndKeepsNewestFirst()
    {
        RecentCompareEntry older;
        older.descriptor.type = ComparePageType::Text;
        older.descriptor.leftPath = QStringLiteral("/tmp/a.txt");
        older.descriptor.rightPath = QStringLiteral("/tmp/b.txt");

        RecentCompareEntry newer = older;
        newer.descriptor.title = QStringLiteral("Updated");
        newer.accessedAt = QDateTime::currentDateTimeUtc();

        RecentCompareEntry different;
        different.descriptor.type = ComparePageType::Folder;
        different.descriptor.leftPath = QStringLiteral("/tmp/left");
        different.descriptor.rightPath = QStringLiteral("/tmp/right");

        const auto updated = upsertRecentCompareEntry({ older, different }, newer);
        QCOMPARE(updated.size(), 2);
        QCOMPARE(updated.at(0).descriptor.title, QStringLiteral("Updated"));
        QCOMPARE(updated.at(0).descriptor.leftPath, QStringLiteral("/tmp/a.txt"));
        QCOMPARE(updated.at(1).descriptor.type, ComparePageType::Folder);
    }

    void removesMatchingDescriptor()
    {
        RecentCompareEntry first;
        first.descriptor.type = ComparePageType::Text;
        first.descriptor.leftPath = QStringLiteral("/tmp/a.txt");
        first.descriptor.rightPath = QStringLiteral("/tmp/b.txt");

        RecentCompareEntry second;
        second.descriptor.type = ComparePageType::Folder;
        second.descriptor.leftPath = QStringLiteral("/tmp/left");
        second.descriptor.rightPath = QStringLiteral("/tmp/right");

        const auto updated = removeRecentCompareEntry({ first, second }, first.descriptor);
        QCOMPARE(updated.size(), 1);
        QCOMPARE(updated.at(0).descriptor.type, ComparePageType::Folder);
        QCOMPARE(updated.at(0).descriptor.leftPath, QStringLiteral("/tmp/left"));
    }
};

QTEST_MAIN(RecentCompareTest)

#include "app_recent_compare_test.moc"

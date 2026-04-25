#include "core/text/lineardiffengine.h"
#include "core/text/textdocumentloader.h"
#include "core/text/textnormalizer.h"

#include <QFileInfo>
#include <QTemporaryFile>
#include <QtTest>

using namespace mergeqt::core;

class CoreTextCompareTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void loaderSplitsLines();
    void loaderReadsFile();
    void loaderDetectsNewlineStyle();
    void normalizerAppliesFlags();
    void diffDetectsReplaceInsertDelete();
    void diffHonorsIgnoreCaseAndWhitespace();
    void diffBuildsDifferenceBlocks();
};

void CoreTextCompareTest::loaderSplitsLines()
{
    TextDocumentLoader loader;
    const TextDocument document = loader.fromText(QStringLiteral("a\nb\n"), QStringLiteral("sample"));

    QCOMPARE(document.label, QStringLiteral("sample"));
    QCOMPARE(document.lines.size(), 3);
    QCOMPARE(document.lines.at(0), QStringLiteral("a"));
    QCOMPARE(document.lines.at(1), QStringLiteral("b"));
    QCOMPARE(document.lines.at(2), QString());
}

void CoreTextCompareTest::loaderReadsFile()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    QVERIFY(file.write("left\nright\n") >= 0);
    file.close();

    TextDocumentLoader loader;
    TextDocument document;
    QString errorMessage;
    QVERIFY(loader.fromFile(file.fileName(), document, errorMessage));
    QCOMPARE(document.sourcePath, file.fileName());
    QCOMPARE(document.label, QFileInfo(file.fileName()).fileName());
    QCOMPARE(document.lines.size(), 3);
}

void CoreTextCompareTest::loaderDetectsNewlineStyle()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    QVERIFY(file.write("left\r\nright\r\n") >= 0);
    file.close();

    TextDocumentLoader loader;
    TextDocument document;
    QString errorMessage;
    QVERIFY(loader.fromFile(file.fileName(), document, errorMessage));
    QCOMPARE(document.newlineStyle, NewlineStyle::CRLF);
    QCOMPARE(document.encodingName, QStringLiteral("UTF-8"));
}

void CoreTextCompareTest::normalizerAppliesFlags()
{
    TextDocumentLoader loader;
    TextNormalizer normalizer;
    const TextDocument document = loader.fromText(QStringLiteral("  Alpha  \nBeta "));

    const NormalizedTextDocument normalized = normalizer.normalize(document, true, true);
    QCOMPARE(normalized.originalLines.at(0), QStringLiteral("  Alpha  "));
    QCOMPARE(normalized.normalizedLines.at(0), QStringLiteral("alpha"));
    QCOMPARE(normalized.normalizedLines.at(1), QStringLiteral("beta"));
}

void CoreTextCompareTest::diffDetectsReplaceInsertDelete()
{
    TextDocumentLoader loader;
    TextNormalizer normalizer;
    MinimalLineDiffEngine engine;

    const TextDocument left = loader.fromText(QStringLiteral("alpha\nbeta\ngamma"));
    const TextDocument right = loader.fromText(QStringLiteral("alpha\nbeta changed\ngamma\ndelta"));
    const auto normalizedLeft = normalizer.normalize(left, false, false);
    const auto normalizedRight = normalizer.normalize(right, false, false);

    const TextCompareResult result = engine.compare(normalizedLeft, normalizedRight);

    QVERIFY(result.hasDifferences());
    QCOMPARE(result.stats.equalCount, 2);
    QCOMPARE(result.stats.replaceCount, 1);
    QCOMPARE(result.stats.insertCount, 1);
    QCOMPARE(result.stats.deleteCount, 0);
}

void CoreTextCompareTest::diffHonorsIgnoreCaseAndWhitespace()
{
    TextDocumentLoader loader;
    TextNormalizer normalizer;
    MinimalLineDiffEngine engine;

    const TextDocument left = loader.fromText(QStringLiteral(" Alpha \nBeta"));
    const TextDocument right = loader.fromText(QStringLiteral("alpha\nbeta"));

    const auto normalizedLeft = normalizer.normalize(left, true, true);
    const auto normalizedRight = normalizer.normalize(right, true, true);
    const TextCompareResult result = engine.compare(normalizedLeft, normalizedRight);

    QVERIFY(!result.hasDifferences());
    QCOMPARE(result.stats.equalCount, 2);
}

void CoreTextCompareTest::diffBuildsDifferenceBlocks()
{
    TextDocumentLoader loader;
    TextNormalizer normalizer;
    MinimalLineDiffEngine engine;

    const TextDocument left = loader.fromText(QStringLiteral("same\nremove\nsame2\nold"));
    const TextDocument right = loader.fromText(QStringLiteral("same\nsame2\nnew"));

    const auto result = engine.compare(normalizer.normalize(left, false, false),
                                       normalizer.normalize(right, false, false));

    QCOMPARE(result.blocks.size(), 5);
    QCOMPARE(result.blocks.at(0).operation, TextDiffOperation::Equal);
    QCOMPARE(result.blocks.at(1).operation, TextDiffOperation::Delete);
    QCOMPARE(result.blocks.at(2).operation, TextDiffOperation::Equal);
    QCOMPARE(result.blocks.at(3).operation, TextDiffOperation::Delete);
    QCOMPARE(result.blocks.at(4).operation, TextDiffOperation::Insert);
}

QTEST_MAIN(CoreTextCompareTest)

#include "core_text_compare_test.moc"

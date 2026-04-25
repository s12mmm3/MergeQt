#include "app/compare/comparetypes.h"
#include "ui/widgets/linenumbertextedit.h"
#include "ui/widgets/textcomparetabpage.h"

#include <QTemporaryFile>
#include <QtTest>

class TextCompareAlignmentTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void compareViewPadsMissingRowsForAlignedDisplay();
};

void TextCompareAlignmentTest::compareViewPadsMissingRowsForAlignedDisplay()
{
    QTemporaryFile leftFile;
    QTemporaryFile rightFile;
    QVERIFY(leftFile.open());
    QVERIFY(rightFile.open());

    QVERIFY(leftFile.write("same\nleft only\nsame2\n") >= 0);
    QVERIFY(rightFile.write("same\nsame2\nright only\n") >= 0);
    leftFile.close();
    rightFile.close();

    mergeqt::app::ComparePageDescriptor descriptor;
    descriptor.type = mergeqt::app::ComparePageType::Text;
    descriptor.leftPath = leftFile.fileName();
    descriptor.rightPath = rightFile.fileName();

    mergeqt::ui::TextCompareTabPage page(descriptor);
    page.resize(1200, 800);
    page.show();
    QVERIFY(QTest::qWaitForWindowExposed(&page));

    auto *leftEditor = page.findChild<mergeqt::ui::LineNumberTextEdit *>(QStringLiteral("leftCompareEditor"));
    auto *rightEditor = page.findChild<mergeqt::ui::LineNumberTextEdit *>(QStringLiteral("rightCompareEditor"));
    QVERIFY(leftEditor);
    QVERIFY(rightEditor);

    const QStringList leftLines = leftEditor->toPlainText().split(QLatin1Char('\n'));
    const QStringList rightLines = rightEditor->toPlainText().split(QLatin1Char('\n'));

    QCOMPARE(leftLines.size(), rightLines.size());
    QVERIFY(leftLines.contains(QStringLiteral("left only")));
    QVERIFY(rightLines.contains(QStringLiteral("right only")));
    QVERIFY(leftLines.contains(QString()));
    QVERIFY(rightLines.contains(QString()));
    QCOMPARE(leftEditor->displayedLineNumberForVisualLine(0), 1);
    QCOMPARE(rightEditor->displayedLineNumberForVisualLine(0), 1);
    QCOMPARE(leftEditor->displayedLineNumberForVisualLine(1), 2);
    QCOMPARE(rightEditor->displayedLineNumberForVisualLine(1), -1);
    QCOMPARE(leftEditor->displayedLineNumberForVisualLine(2), 3);
    QCOMPARE(rightEditor->displayedLineNumberForVisualLine(2), 2);
}

QTEST_MAIN(TextCompareAlignmentTest)

#include "textcompare_alignment_test.moc"

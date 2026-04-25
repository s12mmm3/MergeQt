#include "ui/widgets/linenumbertextedit.h"

#include <QScrollBar>
#include <QtTest>

class LineNumberTextEditSyncTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void sliderMoveSyncsAllRegisteredEditors();
    void horizontalScrollRemainsIndependent();
};

void LineNumberTextEditSyncTest::sliderMoveSyncsAllRegisteredEditors()
{
    QWidget window;
    auto *leftEditor = new mergeqt::ui::LineNumberTextEdit(&window);
    auto *middleEditor = new mergeqt::ui::LineNumberTextEdit(&window);
    auto *rightEditor = new mergeqt::ui::LineNumberTextEdit(&window);

    leftEditor->setGeometry(0, 0, 280, 420);
    middleEditor->setGeometry(300, 0, 280, 420);
    rightEditor->setGeometry(600, 0, 280, 420);

    QStringList lines;
    for (int index = 0; index < 400; ++index)
        lines.append(QStringLiteral("line %1").arg(index));
    const QString text = lines.join(QLatin1Char('\n'));

    leftEditor->setPlainText(text);
    middleEditor->setPlainText(text);
    rightEditor->setPlainText(text);

    leftEditor->addSynchronizedEditor(middleEditor);
    leftEditor->addSynchronizedEditor(rightEditor);

    window.resize(900, 440);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *leftScrollBar = leftEditor->verticalScrollBar();
    auto *middleScrollBar = middleEditor->verticalScrollBar();
    auto *rightScrollBar = rightEditor->verticalScrollBar();
    QVERIFY(leftScrollBar->maximum() > 0);

    const int targetValue = leftScrollBar->maximum() / 2;
    leftScrollBar->setSliderDown(true);
    leftScrollBar->setValue(targetValue);
    emit leftScrollBar->sliderMoved(targetValue);
    leftScrollBar->setSliderDown(false);
    QCoreApplication::processEvents();

    QCOMPARE(middleScrollBar->value(), targetValue);
    QCOMPARE(rightScrollBar->value(), targetValue);
}

void LineNumberTextEditSyncTest::horizontalScrollRemainsIndependent()
{
    QWidget window;
    auto *leftEditor = new mergeqt::ui::LineNumberTextEdit(&window);
    auto *rightEditor = new mergeqt::ui::LineNumberTextEdit(&window);

    leftEditor->setGeometry(0, 0, 320, 240);
    rightEditor->setGeometry(340, 0, 320, 240);

    const QString longLine = QStringLiteral("abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    QStringList lines;
    for (int index = 0; index < 40; ++index)
        lines.append(longLine + longLine + longLine);
    const QString text = lines.join(QLatin1Char('\n'));

    leftEditor->setPlainText(text);
    rightEditor->setPlainText(text);
    leftEditor->addSynchronizedEditor(rightEditor);

    window.resize(680, 260);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *leftHorizontalScrollBar = leftEditor->horizontalScrollBar();
    auto *rightHorizontalScrollBar = rightEditor->horizontalScrollBar();
    QVERIFY(leftHorizontalScrollBar->maximum() > 0);

    const int targetValue = leftHorizontalScrollBar->maximum() / 2;
    leftHorizontalScrollBar->setValue(targetValue);
    QCoreApplication::processEvents();

    QCOMPARE(leftHorizontalScrollBar->value(), targetValue);
    QCOMPARE(rightHorizontalScrollBar->value(), 0);
}

QTEST_MAIN(LineNumberTextEditSyncTest)

#include "linenumbertextedit_sync_test.moc"

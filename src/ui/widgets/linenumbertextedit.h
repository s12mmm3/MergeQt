#pragma once

#include <QPlainTextEdit>
#include <QPointer>
#include <QVector>

class QWidget;
class QPaintEvent;
class QResizeEvent;
class QScrollBar;

namespace mergeqt::ui {

class LineNumberTextEdit;

class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(LineNumberTextEdit *editor);

    [[nodiscard]] QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    LineNumberTextEdit *m_editor = nullptr;
};

class LineNumberTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit LineNumberTextEdit(QWidget *parent = nullptr);

    [[nodiscard]] int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void addSynchronizedEditor(LineNumberTextEdit *editor);
    void removeSynchronizedEditor(LineNumberTextEdit *editor);
    void clearSynchronizedEditors();
    void setDisplayedLineNumbers(const QVector<int> &lineNumbers);
    void clearDisplayedLineNumbers();
    [[nodiscard]] int displayedLineNumberForVisualLine(int visualLineIndex) const;

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
    void syncPeerEditors();
    [[nodiscard]] int effectiveLineNumber(int visualLineIndex) const;

    QWidget *m_lineNumberArea = nullptr;
    QList<QPointer<LineNumberTextEdit>> m_synchronizedEditors;
    QVector<int> m_displayedLineNumbers;
};

} // namespace mergeqt::ui

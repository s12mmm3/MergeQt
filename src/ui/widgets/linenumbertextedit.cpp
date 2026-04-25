#include "ui/widgets/linenumbertextedit.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QTextBlock>

namespace mergeqt::ui {

LineNumberArea::LineNumberArea(LineNumberTextEdit *editor)
    : QWidget(editor)
    , m_editor(editor)
{
}

QSize LineNumberArea::sizeHint() const
{
    return {m_editor->lineNumberAreaWidth(), 0};
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    m_editor->lineNumberAreaPaintEvent(event);
}

LineNumberTextEdit::LineNumberTextEdit(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_lineNumberArea(new LineNumberArea(this))
{
    setLineWrapMode(QPlainTextEdit::NoWrap);

    updateLineNumberAreaWidth(0);

    connect(this, &QPlainTextEdit::blockCountChanged, this, &LineNumberTextEdit::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &LineNumberTextEdit::updateLineNumberArea);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int) {
        syncPeerEditors();
    });
}

int LineNumberTextEdit::lineNumberAreaWidth() const
{
    int digits = 1;
    int max = 1;
    if (!m_displayedLineNumbers.isEmpty()) {
        for (const int lineNumber : m_displayedLineNumbers)
            max = qMax(max, lineNumber);
    } else {
        max = qMax(1, blockCount());
    }
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    return 12 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void LineNumberTextEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), QColor(245, 245, 245));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            const int lineNumber = effectiveLineNumber(blockNumber);
            if (lineNumber > 0) {
                const QString number = QString::number(lineNumber);
                painter.setPen(QColor(120, 120, 120));
                painter.drawText(0, top, m_lineNumberArea->width() - 6, fontMetrics().height(),
                                 Qt::AlignRight, number);
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void LineNumberTextEdit::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    const QRect contentRect = contentsRect();
    m_lineNumberArea->setGeometry(QRect(contentRect.left(), contentRect.top(),
                                        lineNumberAreaWidth(), contentRect.height()));
}

void LineNumberTextEdit::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void LineNumberTextEdit::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void LineNumberTextEdit::addSynchronizedEditor(LineNumberTextEdit *editor)
{
    if (!editor || editor == this || m_synchronizedEditors.contains(editor))
        return;

    m_synchronizedEditors.append(editor);
}

void LineNumberTextEdit::removeSynchronizedEditor(LineNumberTextEdit *editor)
{
    if (!editor)
        return;

    m_synchronizedEditors.removeAll(editor);
}

void LineNumberTextEdit::clearSynchronizedEditors()
{
    m_synchronizedEditors.clear();
}

void LineNumberTextEdit::setDisplayedLineNumbers(const QVector<int> &lineNumbers)
{
    if (m_displayedLineNumbers == lineNumbers)
        return;

    m_displayedLineNumbers = lineNumbers;
    updateLineNumberAreaWidth(0);
    m_lineNumberArea->update();
}

void LineNumberTextEdit::clearDisplayedLineNumbers()
{
    if (m_displayedLineNumbers.isEmpty())
        return;

    m_displayedLineNumbers.clear();
    updateLineNumberAreaWidth(0);
    m_lineNumberArea->update();
}

int LineNumberTextEdit::displayedLineNumberForVisualLine(int visualLineIndex) const
{
    return effectiveLineNumber(visualLineIndex);
}

void LineNumberTextEdit::syncPeerEditors()
{
    auto *sourceScrollBar = verticalScrollBar();
    if (!sourceScrollBar)
        return;

    for (auto it = m_synchronizedEditors.begin(); it != m_synchronizedEditors.end();) {
        LineNumberTextEdit *editor = it->data();
        if (!editor) {
            it = m_synchronizedEditors.erase(it);
            continue;
        }

        auto *targetScrollBar = editor->verticalScrollBar();
        if (targetScrollBar && targetScrollBar->value() != sourceScrollBar->value())
            targetScrollBar->setValue(sourceScrollBar->value());
        ++it;
    }
}

int LineNumberTextEdit::effectiveLineNumber(int visualLineIndex) const
{
    if (visualLineIndex < 0)
        return -1;

    if (visualLineIndex < m_displayedLineNumbers.size())
        return m_displayedLineNumbers.at(visualLineIndex);

    return visualLineIndex + 1;
}

} // namespace mergeqt::ui

#include "ui/widgets/textdiffoverviewbar.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <algorithm>

namespace mergeqt::ui {

TextDiffOverviewBar::TextDiffOverviewBar(QWidget *parent)
    : QWidget(parent)
{
    setMinimumWidth(20);
    setMouseTracking(true);
    setToolTip(tr("Difference overview"));
}

void TextDiffOverviewBar::setCompareResult(const mergeqt::core::TextCompareResult &result)
{
    m_result = result;
    if (m_selectedBlockIndex >= m_result.blocks.size())
        m_selectedBlockIndex = -1;
    if (m_hoveredBlockIndex >= m_result.blocks.size())
        m_hoveredBlockIndex = -1;
    update();
}

void TextDiffOverviewBar::setSelectedBlockIndex(int blockIndex)
{
    if (m_selectedBlockIndex == blockIndex)
        return;

    m_selectedBlockIndex = blockIndex;
    update();
}

void TextDiffOverviewBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.fillRect(rect(), palette().alternateBase());

    const QRect contentRect = rect().adjusted(4, 4, -4, -4);
    if (contentRect.height() <= 0 || m_result.lines.isEmpty()) {
        painter.setPen(palette().mid().color());
        painter.drawRect(contentRect);
        return;
    }

    painter.setPen(Qt::NoPen);
    const int totalLineCount = std::max(1, static_cast<int>(m_result.lines.size()));
    for (int blockIndex = 0; blockIndex < m_result.blocks.size(); ++blockIndex) {
        const auto &block = m_result.blocks.at(blockIndex);
        if (!block.isDifference())
            continue;

        const double startRatio = static_cast<double>(std::max(0, block.startIndex)) / totalLineCount;
        const double endRatio = static_cast<double>(std::max(block.startIndex + 1, block.endIndex + 1)) / totalLineCount;
        int top = contentRect.top() + static_cast<int>(contentRect.height() * startRatio);
        int bottom = contentRect.top() + static_cast<int>(contentRect.height() * endRatio);
        if (bottom <= top)
            bottom = top + 3;

        QRect markerRect(contentRect.left(), top, contentRect.width(), bottom - top);
        if (blockIndex == m_hoveredBlockIndex)
            markerRect.adjust(-1, 0, 1, 0);

        painter.setBrush(blockColor(block.operation, blockIndex == m_selectedBlockIndex));
        painter.drawRoundedRect(markerRect, 2, 2);
    }

    painter.setPen(palette().mid().color());
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(contentRect);
}

void TextDiffOverviewBar::mousePressEvent(QMouseEvent *event)
{
    const int blockIndex = blockIndexAtPosition(event->position().toPoint().y());
    if (blockIndex >= 0)
        emit differenceBlockActivated(blockIndex);
}

void TextDiffOverviewBar::mouseMoveEvent(QMouseEvent *event)
{
    const int blockIndex = blockIndexAtPosition(event->position().toPoint().y());
    if (m_hoveredBlockIndex == blockIndex)
        return;

    m_hoveredBlockIndex = blockIndex;
    update();
}

void TextDiffOverviewBar::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    if (m_hoveredBlockIndex < 0)
        return;

    m_hoveredBlockIndex = -1;
    update();
}

QSize TextDiffOverviewBar::sizeHint() const
{
    return { 24, 160 };
}

int TextDiffOverviewBar::blockIndexAtPosition(int y) const
{
    const QRect contentRect = rect().adjusted(4, 4, -4, -4);
    if (contentRect.height() <= 0 || m_result.lines.isEmpty())
        return -1;

    const int totalLineCount = std::max(1, static_cast<int>(m_result.lines.size()));
    for (int blockIndex = 0; blockIndex < m_result.blocks.size(); ++blockIndex) {
        const auto &block = m_result.blocks.at(blockIndex);
        if (!block.isDifference())
            continue;

        const double startRatio = static_cast<double>(std::max(0, block.startIndex)) / totalLineCount;
        const double endRatio = static_cast<double>(std::max(block.startIndex + 1, block.endIndex + 1)) / totalLineCount;
        int top = contentRect.top() + static_cast<int>(contentRect.height() * startRatio);
        int bottom = contentRect.top() + static_cast<int>(contentRect.height() * endRatio);
        if (bottom <= top)
            bottom = top + 3;

        if (y >= top && y <= bottom)
            return blockIndex;
    }

    return -1;
}

QColor TextDiffOverviewBar::blockColor(mergeqt::core::TextDiffOperation operation, bool selected) const
{
    switch (operation) {
    case mergeqt::core::TextDiffOperation::Insert:
        return selected ? QColor(102, 187, 106) : QColor(165, 214, 167);
    case mergeqt::core::TextDiffOperation::Delete:
        return selected ? QColor(239, 83, 80) : QColor(255, 205, 210);
    case mergeqt::core::TextDiffOperation::Replace:
        return selected ? QColor(255, 167, 38) : QColor(255, 224, 178);
    case mergeqt::core::TextDiffOperation::Equal:
        break;
    }

    return selected ? QColor(120, 144, 156) : QColor(207, 216, 220);
}

} // namespace mergeqt::ui

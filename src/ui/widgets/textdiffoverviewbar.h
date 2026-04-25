#pragma once

#include "core/text/textcomparetypes.h"

#include <QWidget>

namespace mergeqt::ui {

class TextDiffOverviewBar : public QWidget
{
    Q_OBJECT

public:
    explicit TextDiffOverviewBar(QWidget *parent = nullptr);

    void setCompareResult(const mergeqt::core::TextCompareResult &result);
    void setSelectedBlockIndex(int blockIndex);

Q_SIGNALS:
    void differenceBlockActivated(int blockIndex);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    [[nodiscard]] QSize sizeHint() const override;

private:
    [[nodiscard]] int blockIndexAtPosition(int y) const;
    [[nodiscard]] QColor blockColor(mergeqt::core::TextDiffOperation operation, bool selected) const;

    mergeqt::core::TextCompareResult m_result;
    int m_selectedBlockIndex = -1;
    int m_hoveredBlockIndex = -1;
};

} // namespace mergeqt::ui

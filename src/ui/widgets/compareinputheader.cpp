#include "ui/widgets/compareinputheader.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace mergeqt::ui {

CompareInputHeader::CompareInputHeader(const QString &leftLabelText,
                                       const QString &rightLabelText,
                                       QWidget *parent)
    : QWidget(parent)
    , m_leftLabelText(leftLabelText)
    , m_rightLabelText(rightLabelText)
{
    auto *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setHorizontalSpacing(8);
    layout->setVerticalSpacing(6);

    m_leftLabel = new QLabel(m_leftLabelText, this);
    m_rightLabel = new QLabel(m_rightLabelText, this);
    m_leftPathEdit = new QLineEdit(this);
    m_rightPathEdit = new QLineEdit(this);
    m_leftBrowseButton = new QPushButton(this);
    m_rightBrowseButton = new QPushButton(this);
    m_compareButton = new QPushButton(this);
    m_reloadButton = new QPushButton(this);
    m_swapButton = new QPushButton(this);

    layout->addWidget(m_leftLabel, 0, 0);
    layout->addWidget(m_leftPathEdit, 0, 1);
    layout->addWidget(m_leftBrowseButton, 0, 2);
    layout->addWidget(m_rightLabel, 1, 0);
    layout->addWidget(m_rightPathEdit, 1, 1);
    layout->addWidget(m_rightBrowseButton, 1, 2);

    auto *buttonRow = new QWidget(this);
    auto *buttonLayout = new QGridLayout(buttonRow);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setHorizontalSpacing(8);
    buttonLayout->addWidget(m_compareButton, 0, 0);
    buttonLayout->addWidget(m_reloadButton, 0, 1);
    buttonLayout->addWidget(m_swapButton, 0, 2);
    layout->addWidget(buttonRow, 0, 3, 2, 1);

    layout->setColumnStretch(1, 1);

    connect(m_leftBrowseButton, &QPushButton::clicked, this, &CompareInputHeader::leftBrowseRequested);
    connect(m_rightBrowseButton, &QPushButton::clicked, this, &CompareInputHeader::rightBrowseRequested);
    connect(m_compareButton, &QPushButton::clicked, this, &CompareInputHeader::compareRequested);
    connect(m_reloadButton, &QPushButton::clicked, this, &CompareInputHeader::reloadRequested);
    connect(m_swapButton, &QPushButton::clicked, this, &CompareInputHeader::swapRequested);
    retranslateUi();
}

void CompareInputHeader::setLabelTexts(const QString &leftLabelText, const QString &rightLabelText)
{
    m_leftLabelText = leftLabelText;
    m_rightLabelText = rightLabelText;
    if (m_leftLabel)
        m_leftLabel->setText(m_leftLabelText);
    if (m_rightLabel)
        m_rightLabel->setText(m_rightLabelText);
}

void CompareInputHeader::retranslateUi()
{
    setLabelTexts(m_leftLabelText, m_rightLabelText);
    if (m_leftBrowseButton)
        m_leftBrowseButton->setText(tr("Open..."));
    if (m_rightBrowseButton)
        m_rightBrowseButton->setText(tr("Open..."));
    if (m_compareButton)
        m_compareButton->setText(tr("Compare"));
    if (m_reloadButton)
        m_reloadButton->setText(tr("Reload"));
    if (m_swapButton)
        m_swapButton->setText(tr("Swap"));
}

QLineEdit *CompareInputHeader::leftPathEdit() const
{
    return m_leftPathEdit;
}

QLineEdit *CompareInputHeader::rightPathEdit() const
{
    return m_rightPathEdit;
}

QPushButton *CompareInputHeader::compareButton() const
{
    return m_compareButton;
}

QPushButton *CompareInputHeader::reloadButton() const
{
    return m_reloadButton;
}

QPushButton *CompareInputHeader::swapButton() const
{
    return m_swapButton;
}

} // namespace mergeqt::ui

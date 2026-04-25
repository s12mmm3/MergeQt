#include "ui/widgets/placeholdercomparetabpage.h"

#include <QLabel>
#include <QVBoxLayout>

namespace mergeqt::ui {

PlaceholderCompareTabPage::PlaceholderCompareTabPage(mergeqt::app::ComparePageType type, QWidget *parent)
    : CompareTabPage(parent)
    , m_type(type)
{
    auto *layout = new QVBoxLayout(this);
    m_label = new QLabel(this);
    m_label->setWordWrap(true);
    layout->addWidget(m_label);
    layout->addStretch(1);
    retranslateUi();
}

mergeqt::app::ComparePageType PlaceholderCompareTabPage::pageType() const
{
    return m_type;
}

QString PlaceholderCompareTabPage::pageTitle() const
{
    switch (m_type) {
    case mergeqt::app::ComparePageType::Text:
        return tr("Text Compare");
    case mergeqt::app::ComparePageType::Folder:
        return tr("Folder Compare");
    case mergeqt::app::ComparePageType::Binary:
        return tr("Binary Compare");
    case mergeqt::app::ComparePageType::Image:
        return tr("Image Compare");
    }
    return tr("Compare");
}

QString PlaceholderCompareTabPage::pageStatusText() const
{
    return m_statusText;
}

CompareTabCapabilities PlaceholderCompareTabPage::capabilities() const
{
    return {};
}

bool PlaceholderCompareTabPage::chooseLeftFile() { return false; }
bool PlaceholderCompareTabPage::chooseRightFile() { return false; }
bool PlaceholderCompareTabPage::reloadInputs() { return false; }
bool PlaceholderCompareTabPage::swapInputs() { return false; }
bool PlaceholderCompareTabPage::executeCompare() { return false; }
void PlaceholderCompareTabPage::navigateDifferenceByOffset(int) {}
void PlaceholderCompareTabPage::applyWindowSettings() {}
void PlaceholderCompareTabPage::retranslateUi()
{
    if (m_label)
        m_label->setText(tr("This comparison page is reserved for a future compare type."));
    m_statusText = tr("Page scaffold ready");
    emit pageTitleChanged(pageTitle());
    emit pageStatusChanged(pageStatusText());
}

} // namespace mergeqt::ui

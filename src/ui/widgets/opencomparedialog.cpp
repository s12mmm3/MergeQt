#include "ui/widgets/opencomparedialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace mergeqt::ui {

OpenCompareDialog::OpenCompareDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Open Compare"));
    setModal(true);
    resize(720, 220);
    buildUi();
    connectSignals();
    updateLabelsForCurrentType();
}

mergeqt::app::ComparePageDescriptor OpenCompareDialog::descriptor() const
{
    mergeqt::app::ComparePageDescriptor descriptor;
    descriptor.type = static_cast<mergeqt::app::ComparePageType>(m_typeCombo->currentData().toInt());
    descriptor.title = m_titleEdit->text().trimmed();
    descriptor.leftPath = m_leftPathEdit->text().trimmed();
    descriptor.rightPath = m_rightPathEdit->text().trimmed();
    return descriptor;
}

void OpenCompareDialog::buildUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(12, 12, 12, 12);
    rootLayout->setSpacing(10);

    auto *summaryLabel = new QLabel(
        tr("Choose a compare type and the left/right inputs. This dialog is the unified entry point for opening new compare tabs."),
        this);
    summaryLabel->setWordWrap(true);
    rootLayout->addWidget(summaryLabel);

    auto *formLayout = new QFormLayout();
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem(tr("Text Compare"), static_cast<int>(mergeqt::app::ComparePageType::Text));
    m_typeCombo->addItem(tr("Folder Compare"), static_cast<int>(mergeqt::app::ComparePageType::Folder));
    formLayout->addRow(tr("Compare type"), m_typeCombo);

    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setPlaceholderText(tr("Optional custom tab title"));
    formLayout->addRow(tr("Tab title"), m_titleEdit);

    auto *leftRow = new QWidget(this);
    auto *leftLayout = new QHBoxLayout(leftRow);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    m_leftPathEdit = new QLineEdit(leftRow);
    m_leftBrowseButton = new QPushButton(tr("Browse..."), leftRow);
    leftLayout->addWidget(m_leftPathEdit);
    leftLayout->addWidget(m_leftBrowseButton);
    formLayout->addRow(tr("Left input"), leftRow);

    auto *rightRow = new QWidget(this);
    auto *rightLayout = new QHBoxLayout(rightRow);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    m_rightPathEdit = new QLineEdit(rightRow);
    m_rightBrowseButton = new QPushButton(tr("Browse..."), rightRow);
    rightLayout->addWidget(m_rightPathEdit);
    rightLayout->addWidget(m_rightBrowseButton);
    formLayout->addRow(tr("Right input"), rightRow);

    rootLayout->addLayout(formLayout);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    rootLayout->addWidget(m_buttonBox);
}

void OpenCompareDialog::connectSignals()
{
    connect(m_typeCombo, &QComboBox::currentIndexChanged, this, [this](int) { updateLabelsForCurrentType(); });
    connect(m_leftBrowseButton, &QPushButton::clicked, this, &OpenCompareDialog::chooseLeftPath);
    connect(m_rightBrowseButton, &QPushButton::clicked, this, &OpenCompareDialog::chooseRightPath);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void OpenCompareDialog::updateLabelsForCurrentType()
{
    const auto type = static_cast<mergeqt::app::ComparePageType>(m_typeCombo->currentData().toInt());
    if (type == mergeqt::app::ComparePageType::Folder) {
        m_leftPathEdit->setPlaceholderText(tr("Choose the left folder"));
        m_rightPathEdit->setPlaceholderText(tr("Choose the right folder"));
        return;
    }

    m_leftPathEdit->setPlaceholderText(tr("Choose the left file"));
    m_rightPathEdit->setPlaceholderText(tr("Choose the right file"));
}

void OpenCompareDialog::chooseLeftPath()
{
    const auto type = static_cast<mergeqt::app::ComparePageType>(m_typeCombo->currentData().toInt());
    const QString path = type == mergeqt::app::ComparePageType::Folder
                             ? QFileDialog::getExistingDirectory(this, tr("Open left folder"), m_leftPathEdit->text())
                             : QFileDialog::getOpenFileName(this, tr("Open left file"), m_leftPathEdit->text());
    if (!path.isEmpty())
        m_leftPathEdit->setText(path);
}

void OpenCompareDialog::chooseRightPath()
{
    const auto type = static_cast<mergeqt::app::ComparePageType>(m_typeCombo->currentData().toInt());
    const QString path = type == mergeqt::app::ComparePageType::Folder
                             ? QFileDialog::getExistingDirectory(this, tr("Open right folder"), m_rightPathEdit->text())
                             : QFileDialog::getOpenFileName(this, tr("Open right file"), m_rightPathEdit->text());
    if (!path.isEmpty())
        m_rightPathEdit->setText(path);
}

} // namespace mergeqt::ui

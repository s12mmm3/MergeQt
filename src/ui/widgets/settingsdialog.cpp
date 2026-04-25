#include "ui/widgets/settingsdialog.h"

#include "app/settings/settingsservice.h"
#include "app/application/appcontext.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLocale>
#include <QStyleFactory>
#include <QVBoxLayout>

namespace mergeqt::ui {

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Settings"));
    resize(560, 360);
    buildUi();
    loadSettings();
}

void SettingsDialog::buildUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(12, 12, 12, 12);
    rootLayout->setSpacing(12);

    auto *uiGroup = new QGroupBox(tr("Interface"), this);
    auto *uiLayout = new QFormLayout(uiGroup);
    m_languageCombo = new QComboBox(uiGroup);
    m_languageCombo->addItem(tr("System Default"), QStringLiteral("system"));
    const QStringList availableLanguages = mergeqt::app::AppContext::instance()->availableLanguages();
    for (const QString &languageTag : availableLanguages) {
        const QLocale locale(languageTag);
        const QString label = locale.nativeLanguageName();
        m_languageCombo->addItem(label.isEmpty() ? languageTag : label, languageTag);
    }
    m_styleCombo = new QComboBox(uiGroup);
    m_styleCombo->addItem(tr("System Default"), QString());
    const QStringList styleKeys = QStyleFactory::keys();
    for (const QString &styleKey : styleKeys)
        m_styleCombo->addItem(styleKey, styleKey);
    m_colorSchemeCombo = new QComboBox(uiGroup);
    m_colorSchemeCombo->addItem(tr("Follow System"), -1);
    m_colorSchemeCombo->addItem(tr("Light"), 0);
    m_colorSchemeCombo->addItem(tr("Dark"), 1);
    uiLayout->addRow(tr("Language"), m_languageCombo);
    uiLayout->addRow(tr("Style"), m_styleCombo);
    uiLayout->addRow(tr("Color scheme"), m_colorSchemeCombo);
    rootLayout->addWidget(uiGroup);

    auto *compareGroup = new QGroupBox(tr("Compare"), this);
    auto *compareLayout = new QVBoxLayout(compareGroup);
    m_ignoreCaseCheck = new QCheckBox(tr("Ignore case"), compareGroup);
    m_trimWhitespaceCheck = new QCheckBox(tr("Trim whitespace"), compareGroup);
    m_showIdenticalItemsCheck = new QCheckBox(tr("Show identical items"), compareGroup);
    compareLayout->addWidget(m_ignoreCaseCheck);
    compareLayout->addWidget(m_trimWhitespaceCheck);
    compareLayout->addWidget(m_showIdenticalItemsCheck);
    rootLayout->addWidget(compareGroup);

    auto *storageGroup = new QGroupBox(tr("Storage"), this);
    auto *storageLayout = new QVBoxLayout(storageGroup);
    m_settingsPathLabel = new QLabel(storageGroup);
    m_settingsPathLabel->setWordWrap(true);
    storageLayout->addWidget(m_settingsPathLabel);
    rootLayout->addWidget(storageGroup);
    rootLayout->addStretch(1);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    rootLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        applySettings();
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void SettingsDialog::loadSettings()
{
    auto *settings = mergeqt::app::SettingsService::instance();
    const int languageIndex = qMax(0, m_languageCombo->findData(settings->ui()->language()));
    m_languageCombo->setCurrentIndex(languageIndex);
    const int styleIndex = qMax(0, m_styleCombo->findData(settings->ui()->style()));
    m_styleCombo->setCurrentIndex(styleIndex);
    const int colorSchemeIndex = qMax(0, m_colorSchemeCombo->findData(settings->ui()->colorScheme()));
    m_colorSchemeCombo->setCurrentIndex(colorSchemeIndex);
    m_ignoreCaseCheck->setChecked(settings->compare()->ignoreCase());
    m_trimWhitespaceCheck->setChecked(settings->compare()->trimWhitespace());
    m_showIdenticalItemsCheck->setChecked(settings->compare()->showIdenticalItems());
    m_settingsPathLabel->setText(tr("Settings file: %1").arg(settings->settingsFilePath()));
}

void SettingsDialog::applySettings()
{
    auto *settings = mergeqt::app::SettingsService::instance();
    settings->ui()->set_language(m_languageCombo->currentData().toString());
    settings->ui()->set_style(m_styleCombo->currentData().toString());
    settings->ui()->set_colorScheme(m_colorSchemeCombo->currentData().toInt());
    settings->compare()->set_ignoreCase(m_ignoreCaseCheck->isChecked());
    settings->compare()->set_trimWhitespace(m_trimWhitespaceCheck->isChecked());
    settings->compare()->set_showIdenticalItems(m_showIdenticalItemsCheck->isChecked());
}

} // namespace mergeqt::ui

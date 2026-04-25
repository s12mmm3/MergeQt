#include "app/application/appcontext.h"

#include "app/logging/logger.h"
#include "app/settings/settingsservice.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLocale>

namespace mergeqt::app {

AppContext *AppContext::instance()
{
    static AppContext context;
    return &context;
}

QString AppContext::currentPage() const
{
    return m_currentPage;
}

void AppContext::setCurrentPage(const QString &page)
{
    if (m_currentPage == page)
        return;

    m_currentPage = page;
    emit currentPageChanged();
}

QString AppContext::statusMessage() const
{
    return m_statusMessage;
}

void AppContext::setStatusMessage(const QString &message)
{
    if (m_statusMessage == message)
        return;

    m_statusMessage = message;
    emit statusMessageChanged();
}

QString AppContext::currentLanguage() const
{
    return m_currentLanguage;
}

QStringList AppContext::availableLanguages() const
{
    const QString scanDir = QStringLiteral(":/i18n/");
    QStringList result;
    const QFileInfoList fileInfos =
        QDir(scanDir).entryInfoList(QStringList() << QStringLiteral("%1_*.qm").arg(QStringLiteral(PROJECT_NAME)), QDir::Files);

    for (const QFileInfo &fileInfo : fileInfos) {
        QString name = fileInfo.baseName();
        name.remove(QStringLiteral("%1_").arg(QStringLiteral(PROJECT_NAME)));
        if (!name.isEmpty())
            result.push_back(name);
    }

    result.removeDuplicates();
    result.sort(Qt::CaseInsensitive);
    return result;
}

void AppContext::switchLanguage(const QString &languageTag)
{
    const QString normalized = languageTag.isEmpty() ? QStringLiteral("system") : languageTag;
    QLocale locale(normalized == QStringLiteral("system") ? QLocale::system() : QLocale(normalized));
    QLocale::setDefault(locale);

    QCoreApplication::removeTranslator(&m_translator);
    const bool loaded = m_translator.load(locale, QStringLiteral(PROJECT_NAME), QStringLiteral("_"), QStringLiteral(":/i18n"));
    if (loaded)
        QCoreApplication::installTranslator(&m_translator);

    setCurrentLanguage(normalized == QStringLiteral("system") ? locale.name() : normalized);
    SettingsService::instance()->ui()->set_language(normalized);

    LOG_INFO << "UI language set to" << m_currentLanguage << "(translation loaded:" << loaded << ")";
}

void AppContext::setSystemLanguage()
{
    const QString configuredLanguage = SettingsService::instance()->ui()->language();
    if (!configuredLanguage.isEmpty() && configuredLanguage != QStringLiteral("system")) {
        switchLanguage(configuredLanguage);
        return;
    }

    switchLanguage(QStringLiteral("system"));
}

AppContext::AppContext(QObject *parent)
    : QObject(parent)
{
}

void AppContext::setCurrentLanguage(const QString &languageTag)
{
    if (m_currentLanguage == languageTag)
        return;

    m_currentLanguage = languageTag;
    emit currentLanguageChanged();
}

} // namespace mergeqt::app

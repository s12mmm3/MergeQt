#pragma once

#include <QObject>
#include <QString>
#include <QTranslator>

namespace mergeqt::app {

class AppContext : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage WRITE setStatusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString currentLanguage READ currentLanguage NOTIFY currentLanguageChanged)

public:
    static AppContext *instance();

    QString currentPage() const;
    void setCurrentPage(const QString &page);

    QString statusMessage() const;
    void setStatusMessage(const QString &message);

    QString currentLanguage() const;
    QStringList availableLanguages() const;

    Q_INVOKABLE void switchLanguage(const QString &languageTag);
    Q_INVOKABLE void setSystemLanguage();

Q_SIGNALS:
    void currentPageChanged();
    void statusMessageChanged();
    void currentLanguageChanged();

private:
    explicit AppContext(QObject *parent = nullptr);
    void setCurrentLanguage(const QString &languageTag);

    QString m_currentPage = QStringLiteral("overview");
    QString m_statusMessage;
    QString m_currentLanguage = QStringLiteral("system");
    QTranslator m_translator;
};

} // namespace mergeqt::app

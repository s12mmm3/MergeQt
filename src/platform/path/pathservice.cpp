#include "platform/path/pathservice.h"

#include <QDir>
#include <QSettings>
#include <QStandardPaths>

namespace mergeqt::platform {

PathService *PathService::instance()
{
    static PathService service;
    return &service;
}

QString PathService::configDirPath() const
{
    QString path = writableAppPath(QStandardPaths::AppConfigLocation, writableAppPath(QStandardPaths::AppDataLocation));
    if (path.isEmpty())
        path = QDir::homePath();
    ensureDir(path);
    return path;
}

QString PathService::dataDirPath() const
{
    QString path = writableAppPath(QStandardPaths::AppDataLocation, configDirPath());
    if (path.isEmpty())
        path = configDirPath();
    ensureDir(path);
    return path;
}

QString PathService::cacheDirPath() const
{
    QString path = writableAppPath(QStandardPaths::CacheLocation, QDir(tempDirPath()).filePath(QStringLiteral("cache")));
    if (path.isEmpty())
        path = QDir(tempDirPath()).filePath(QStringLiteral("cache"));
    ensureDir(path);
    return path;
}

QString PathService::tempDirPath() const
{
    QString path = writableAppPath(QStandardPaths::TempLocation, QDir::tempPath());
    if (path.isEmpty())
        path = QDir::tempPath();
    ensureDir(path);
    return path;
}

QString PathService::logsDirPath() const
{
    const QString path = QDir(dataDirPath()).filePath(QStringLiteral("logs"));
    ensureDir(path);
    return path;
}

QString PathService::settingsFilePath() const
{
    QSettings settings;
    return settings.fileName();
}

bool PathService::ensureDir(const QString &path) const
{
    if (path.isEmpty())
        return false;

    QDir dir(path);
    return dir.exists() || dir.mkpath(QStringLiteral("."));
}

PathService::PathService(QObject *parent)
    : QObject(parent)
{
}

QString PathService::writableAppPath(int location, const QString &fallback) const
{
    QString path = QStandardPaths::writableLocation(static_cast<QStandardPaths::StandardLocation>(location));
    if (path.isEmpty())
        path = fallback;
    return path;
}

} // namespace mergeqt::platform

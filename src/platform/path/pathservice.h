#pragma once

#include <QObject>
#include <QString>

namespace mergeqt::platform {

class PathService : public QObject
{
    Q_OBJECT

public:
    static PathService *instance();

    Q_INVOKABLE QString configDirPath() const;
    Q_INVOKABLE QString dataDirPath() const;
    Q_INVOKABLE QString cacheDirPath() const;
    Q_INVOKABLE QString tempDirPath() const;
    Q_INVOKABLE QString logsDirPath() const;
    Q_INVOKABLE QString settingsFilePath() const;

    bool ensureDir(const QString &path) const;

private:
    explicit PathService(QObject *parent = nullptr);
    QString writableAppPath(int location, const QString &fallback = {}) const;
};

} // namespace mergeqt::platform

#pragma once

#include "app/settings/propertyhelper.h"
#include "app/compare/comparetypes.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

namespace mergeqt::app {

class UiSettings : public QObject
{
    Q_OBJECT
    MERGEQT_DEFINE_VALUE(QString, style, {})
    MERGEQT_DEFINE_VALUE(QString, language, QStringLiteral("system"))
    MERGEQT_DEFINE_VALUE(int, colorScheme, -1)
    MERGEQT_DEFINE_VALUE(int, windowWidth, 1360)
    MERGEQT_DEFINE_VALUE(int, windowHeight, 900)

public:
    explicit UiSettings(QObject *parent = nullptr);
};

class CompareSettings : public QObject
{
    Q_OBJECT
    MERGEQT_DEFINE_VALUE(bool, ignoreCase, false)
    MERGEQT_DEFINE_VALUE(bool, trimWhitespace, false)
    MERGEQT_DEFINE_VALUE(bool, showIdenticalItems, true)

public:
    explicit CompareSettings(QObject *parent = nullptr);
};

class RuntimeSettings : public QObject
{
    Q_OBJECT
    MERGEQT_DEFINE_VALUE(QString, lastLeftPath, {})
    MERGEQT_DEFINE_VALUE(QString, lastRightPath, {})
    MERGEQT_DEFINE_VALUE(QString, lastLeftFolderPath, {})
    MERGEQT_DEFINE_VALUE(QString, lastRightFolderPath, {})
    MERGEQT_DEFINE_VALUE(QString, lastWorkspace, {})
    MERGEQT_DEFINE_VALUE(QStringList, recentCompareEntries, {})

public:
    explicit RuntimeSettings(QObject *parent = nullptr);
};

class SettingsService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(mergeqt::app::UiSettings *ui READ ui CONSTANT)
    Q_PROPERTY(mergeqt::app::CompareSettings *compare READ compare CONSTANT)
    Q_PROPERTY(mergeqt::app::RuntimeSettings *runtime READ runtime CONSTANT)

public:
    static SettingsService *instance();

    UiSettings *ui();
    CompareSettings *compare();
    RuntimeSettings *runtime();

    Q_INVOKABLE QString settingsFilePath() const;
    [[nodiscard]] QVector<mergeqt::app::RecentCompareEntry> recentCompares() const;
    Q_INVOKABLE void recordRecentCompare(const mergeqt::app::ComparePageDescriptor &descriptor);
    Q_INVOKABLE void removeRecentCompare(const mergeqt::app::ComparePageDescriptor &descriptor);
    Q_INVOKABLE void clearRecentCompares();
    Q_INVOKABLE void sync();

private:
    explicit SettingsService(QObject *parent = nullptr);

    void load();
    void connectPersistence();

    UiSettings m_ui;
    CompareSettings m_compare;
    RuntimeSettings m_runtime;
};

} // namespace mergeqt::app

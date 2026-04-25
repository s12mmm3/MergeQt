#include "app/settings/settingsservice.h"

#include "app/logging/logger.h"
#include "platform/path/pathservice.h"

#include <QSettings>
#include <QSignalBlocker>

namespace {

constexpr auto kUiGroup = "Ui";
constexpr auto kCompareGroup = "Compare";
constexpr auto kRuntimeGroup = "Runtime";

constexpr auto kStyleKey = "Style";
constexpr auto kLanguageKey = "Language";
constexpr auto kColorSchemeKey = "ColorScheme";
constexpr auto kWindowWidthKey = "WindowWidth";
constexpr auto kWindowHeightKey = "WindowHeight";

constexpr auto kIgnoreCaseKey = "IgnoreCase";
constexpr auto kTrimWhitespaceKey = "TrimWhitespace";
constexpr auto kShowIdenticalItemsKey = "ShowIdenticalItems";

constexpr auto kLastLeftPathKey = "LastLeftPath";
constexpr auto kLastRightPathKey = "LastRightPath";
constexpr auto kLastLeftFolderPathKey = "LastLeftFolderPath";
constexpr auto kLastRightFolderPathKey = "LastRightFolderPath";
constexpr auto kLastWorkspaceKey = "LastWorkspace";
constexpr auto kRecentCompareEntriesKey = "RecentCompareEntries";

} // namespace

namespace mergeqt::app {

UiSettings::UiSettings(QObject *parent)
    : QObject(parent)
{
}

CompareSettings::CompareSettings(QObject *parent)
    : QObject(parent)
{
}

RuntimeSettings::RuntimeSettings(QObject *parent)
    : QObject(parent)
{
}

SettingsService *SettingsService::instance()
{
    static SettingsService service;
    return &service;
}

UiSettings *SettingsService::ui()
{
    return &m_ui;
}

CompareSettings *SettingsService::compare()
{
    return &m_compare;
}

RuntimeSettings *SettingsService::runtime()
{
    return &m_runtime;
}

QVector<mergeqt::app::RecentCompareEntry> SettingsService::recentCompares() const
{
    return {};
}

QString SettingsService::settingsFilePath() const
{
    return mergeqt::platform::PathService::instance()->settingsFilePath();
}

void SettingsService::recordRecentCompare(const mergeqt::app::ComparePageDescriptor &descriptor)
{
    Q_UNUSED(descriptor);
}

void SettingsService::removeRecentCompare(const mergeqt::app::ComparePageDescriptor &descriptor)
{
    Q_UNUSED(descriptor);
}

void SettingsService::clearRecentCompares()
{
    m_runtime.set_recentCompareEntries({});
}

void SettingsService::sync()
{
    QSettings settings;

    settings.beginGroup(QLatin1String(kUiGroup));
    settings.setValue(QLatin1String(kStyleKey), m_ui.style());
    settings.setValue(QLatin1String(kLanguageKey), m_ui.language());
    settings.setValue(QLatin1String(kColorSchemeKey), m_ui.colorScheme());
    settings.setValue(QLatin1String(kWindowWidthKey), m_ui.windowWidth());
    settings.setValue(QLatin1String(kWindowHeightKey), m_ui.windowHeight());
    settings.endGroup();

    settings.beginGroup(QLatin1String(kCompareGroup));
    settings.setValue(QLatin1String(kIgnoreCaseKey), m_compare.ignoreCase());
    settings.setValue(QLatin1String(kTrimWhitespaceKey), m_compare.trimWhitespace());
    settings.setValue(QLatin1String(kShowIdenticalItemsKey), m_compare.showIdenticalItems());
    settings.endGroup();

    settings.beginGroup(QLatin1String(kRuntimeGroup));
    settings.setValue(QLatin1String(kLastLeftPathKey), m_runtime.lastLeftPath());
    settings.setValue(QLatin1String(kLastRightPathKey), m_runtime.lastRightPath());
    settings.setValue(QLatin1String(kLastLeftFolderPathKey), m_runtime.lastLeftFolderPath());
    settings.setValue(QLatin1String(kLastRightFolderPathKey), m_runtime.lastRightFolderPath());
    settings.setValue(QLatin1String(kLastWorkspaceKey), m_runtime.lastWorkspace());
    settings.setValue(QLatin1String(kRecentCompareEntriesKey), m_runtime.recentCompareEntries());
    settings.endGroup();
}

SettingsService::SettingsService(QObject *parent)
    : QObject(parent)
    , m_ui(this)
    , m_compare(this)
    , m_runtime(this)
{
    LOG_INFO << "Settings file:" << settingsFilePath();
    load();
    connectPersistence();
}

void SettingsService::load()
{
    QSettings settings;

    {
        QSignalBlocker blocker(&m_ui);
        settings.beginGroup(QLatin1String(kUiGroup));
        m_ui.set_style(settings.value(QLatin1String(kStyleKey), m_ui.style()).toString());
        m_ui.set_language(settings.value(QLatin1String(kLanguageKey), m_ui.language()).toString());
        m_ui.set_colorScheme(settings.value(QLatin1String(kColorSchemeKey), m_ui.colorScheme()).toInt());
        m_ui.set_windowWidth(settings.value(QLatin1String(kWindowWidthKey), m_ui.windowWidth()).toInt());
        m_ui.set_windowHeight(settings.value(QLatin1String(kWindowHeightKey), m_ui.windowHeight()).toInt());
        settings.endGroup();
    }

    {
        QSignalBlocker blocker(&m_compare);
        settings.beginGroup(QLatin1String(kCompareGroup));
        m_compare.set_ignoreCase(settings.value(QLatin1String(kIgnoreCaseKey), m_compare.ignoreCase()).toBool());
        m_compare.set_trimWhitespace(settings.value(QLatin1String(kTrimWhitespaceKey), m_compare.trimWhitespace()).toBool());
        m_compare.set_showIdenticalItems(settings.value(QLatin1String(kShowIdenticalItemsKey), m_compare.showIdenticalItems()).toBool());
        settings.endGroup();
    }

    {
        QSignalBlocker blocker(&m_runtime);
        settings.beginGroup(QLatin1String(kRuntimeGroup));
        m_runtime.set_lastLeftPath(settings.value(QLatin1String(kLastLeftPathKey), m_runtime.lastLeftPath()).toString());
        m_runtime.set_lastRightPath(settings.value(QLatin1String(kLastRightPathKey), m_runtime.lastRightPath()).toString());
        m_runtime.set_lastLeftFolderPath(
            settings.value(QLatin1String(kLastLeftFolderPathKey), m_runtime.lastLeftFolderPath()).toString());
        m_runtime.set_lastRightFolderPath(
            settings.value(QLatin1String(kLastRightFolderPathKey), m_runtime.lastRightFolderPath()).toString());
        m_runtime.set_lastWorkspace(settings.value(QLatin1String(kLastWorkspaceKey), m_runtime.lastWorkspace()).toString());
        m_runtime.set_recentCompareEntries(
            settings.value(QLatin1String(kRecentCompareEntriesKey), m_runtime.recentCompareEntries()).toStringList());
        m_runtime.set_recentCompareEntries({});
        settings.endGroup();
    }
}

void SettingsService::connectPersistence()
{
    connect(&m_ui, &UiSettings::styleChanged, this, &SettingsService::sync);
    connect(&m_ui, &UiSettings::languageChanged, this, &SettingsService::sync);
    connect(&m_ui, &UiSettings::colorSchemeChanged, this, &SettingsService::sync);
    connect(&m_ui, &UiSettings::windowWidthChanged, this, &SettingsService::sync);
    connect(&m_ui, &UiSettings::windowHeightChanged, this, &SettingsService::sync);

    connect(&m_compare, &CompareSettings::ignoreCaseChanged, this, &SettingsService::sync);
    connect(&m_compare, &CompareSettings::trimWhitespaceChanged, this, &SettingsService::sync);
    connect(&m_compare, &CompareSettings::showIdenticalItemsChanged, this, &SettingsService::sync);

    connect(&m_runtime, &RuntimeSettings::lastLeftPathChanged, this, &SettingsService::sync);
    connect(&m_runtime, &RuntimeSettings::lastRightPathChanged, this, &SettingsService::sync);
    connect(&m_runtime, &RuntimeSettings::lastLeftFolderPathChanged, this, &SettingsService::sync);
    connect(&m_runtime, &RuntimeSettings::lastRightFolderPathChanged, this, &SettingsService::sync);
    connect(&m_runtime, &RuntimeSettings::lastWorkspaceChanged, this, &SettingsService::sync);
    connect(&m_runtime, &RuntimeSettings::recentCompareEntriesChanged, this, &SettingsService::sync);
}

} // namespace mergeqt::app

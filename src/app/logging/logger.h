#pragma once

#include <QElapsedTimer>
#include <QLoggingCategory>
#include <QString>

Q_DECLARE_LOGGING_CATEGORY(mergeQtLog)

#define LOG_DEBUG qCDebug(mergeQtLog)
#define LOG_INFO qCInfo(mergeQtLog)
#define LOG_WARNING qCWarning(mergeQtLog)
#define LOG_CRITICAL qCCritical(mergeQtLog)

namespace mergeqt::app {

class ScopedLogTimer
{
public:
    explicit ScopedLogTimer(QString label);
    ~ScopedLogTimer();

private:
    QString m_label;
    QElapsedTimer m_timer;
};

} // namespace mergeqt::app

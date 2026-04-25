#include "app/logging/logger.h"

Q_LOGGING_CATEGORY(mergeQtLog, PROJECT_NAME)

namespace mergeqt::app {

ScopedLogTimer::ScopedLogTimer(QString label)
    : m_label(std::move(label))
{
    m_timer.start();
    LOG_DEBUG.noquote() << m_label << "started";
}

ScopedLogTimer::~ScopedLogTimer()
{
    LOG_DEBUG.noquote() << m_label << "finished in" << m_timer.elapsed() << "ms";
}

} // namespace mergeqt::app

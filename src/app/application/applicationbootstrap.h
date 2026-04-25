#pragma once

#include "app/application/applicationlaunchrequest.h"

class QCoreApplication;

namespace mergeqt::app {

class ApplicationBootstrap
{
public:
    [[nodiscard]] static ApplicationLaunchRequest initialize(QCoreApplication &app);
};

} // namespace mergeqt::app

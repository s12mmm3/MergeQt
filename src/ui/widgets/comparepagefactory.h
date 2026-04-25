#pragma once

#include "app/compare/comparetypes.h"

class QObject;
class QWidget;

namespace mergeqt::ui {

class CompareTabPage;

[[nodiscard]] CompareTabPage *createComparePage(const mergeqt::app::ComparePageDescriptor &descriptor, QWidget *parent = nullptr);
[[nodiscard]] QString defaultComparePageTitle(mergeqt::app::ComparePageType type, const QObject *translationContext = nullptr);

} // namespace mergeqt::ui

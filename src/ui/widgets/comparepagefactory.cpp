#include "ui/widgets/comparepagefactory.h"

#include "ui/widgets/binarycomparetabpage.h"
#include "ui/widgets/foldercomparetabpage.h"
#include "ui/widgets/imagecomparetabpage.h"
#include "ui/widgets/textcomparetabpage.h"

#include <QApplication>
#include <QObject>

namespace mergeqt::ui {

CompareTabPage *createComparePage(const mergeqt::app::ComparePageDescriptor &descriptor, QWidget *parent)
{
    switch (descriptor.type) {
    case mergeqt::app::ComparePageType::Text:
        return new TextCompareTabPage(descriptor, parent);
    case mergeqt::app::ComparePageType::Folder:
        return new FolderCompareTabPage(descriptor, parent);
    case mergeqt::app::ComparePageType::Binary:
        return new BinaryCompareTabPage(descriptor, parent);
    case mergeqt::app::ComparePageType::Image:
        return new ImageCompareTabPage(descriptor, parent);
    }

    return new TextCompareTabPage(descriptor, parent);
}

QString defaultComparePageTitle(mergeqt::app::ComparePageType type, const QObject *translationContext)
{
    const QObject *context = translationContext ? translationContext : qApp;
    switch (type) {
    case mergeqt::app::ComparePageType::Text:
        return context->tr("Text Compare");
    case mergeqt::app::ComparePageType::Folder:
        return context->tr("Folder Compare");
    case mergeqt::app::ComparePageType::Binary:
        return context->tr("Binary Compare");
    case mergeqt::app::ComparePageType::Image:
        return context->tr("Image Compare");
    }

    return context->tr("Compare");
}

} // namespace mergeqt::ui

#include "mainWindow/kddwCustom.h"

#include <kddockwidgets/core/Separator.h>

MySeparator::MySeparator(KDDockWidgets::Core::Separator *controller, KDDockWidgets::Core::View *parent)
    : Separator(controller, parent)
{
    if (controller->isVertical()) {
        View::setFixedWidth(2);
    } else {
        View::setFixedHeight(2);
    }
}

KDDockWidgets::Core::View *
CustomWidgetFactory::createSeparator(KDDockWidgets::Core::Separator *controller,
                                     KDDockWidgets::Core::View *parent) const
{
    return new MySeparator(controller, parent);
}

#include "mainWindow/kddwCustom.h"

#include <QPainter>

MySeparator::MySeparator(KDDockWidgets::Core::Separator *controller, KDDockWidgets::Core::View *parent)
    : Separator(controller, parent) {
}

void MySeparator::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.fillRect(QWidget::rect(), "#ebebeb");
}

KDDockWidgets::Core::View *CustomWidgetFactory::createSeparator(KDDockWidgets::Core::Separator *controller, KDDockWidgets::Core::View *parent) const {
    return new MySeparator(controller, parent);
}

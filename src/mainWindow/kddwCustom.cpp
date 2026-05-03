#include "mainWindow/kddwCustom.h"

#include <QPainter>

#include "globals.h"

CustomWidgetFactory::CustomWidgetFactory(const int theme) {
    f_theme = theme;
}

KDDockWidgets::Core::View * CustomWidgetFactory::createStack(KDDockWidgets::Core::Stack *controller, KDDockWidgets::Core::View *parent) const {
    return new MyStack(controller, KDDockWidgets::QtCommon::View_qt::asQWidget(parent));
}

KDDockWidgets::Core::View *CustomWidgetFactory::createSeparator(KDDockWidgets::Core::Separator *controller, KDDockWidgets::Core::View *parent) const {
    return new MySeparator(controller, parent);
}

MyStack::MyStack(KDDockWidgets::Core::Stack *controller, QWidget *parent)
    : Stack(controller, parent) {
}

void MyStack::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    if (f_theme == Theme::Light) p.fillRect(QWidget::rect(), "#ffffff");
    else p.fillRect(QWidget::rect(), "#242424");
}

MySeparator::MySeparator(KDDockWidgets::Core::Separator *controller, KDDockWidgets::Core::View *parent)
    : Separator(controller, parent) {
}

void MySeparator::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    if (f_theme == Theme::Light) p.fillRect(QWidget::rect(), "#f5f5f5");
    else p.fillRect(QWidget::rect(), "#3d3d3d");
}

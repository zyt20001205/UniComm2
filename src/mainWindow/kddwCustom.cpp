#include "mainWindow/kddwCustom.h"

#include <QPainter>

#include "document/documentModule.h"
#include "document/page/basePage.h"
#include "globals.h"

namespace {
int f_theme{};
}

KDDockWidgets::Core::DockWidget *dockWidgetFactory(const QString &uniqueName) {
    const QUrl documentUrl(uniqueName);
    if (!g_document || !documentUrl.isLocalFile()) return nullptr;
    if (auto *page = g_document->documentConstruct(documentUrl)) return page->dockWidget();
    return nullptr;
}

CustomWidgetFactory::CustomWidgetFactory() {
    f_theme = g_mainConfig["theme"].toInt();
}

KDDockWidgets::Core::View * CustomWidgetFactory::createStack(KDDockWidgets::Core::Stack *controller, KDDockWidgets::Core::View *parent) const {
    return new MyStack(controller, KDDockWidgets::QtCommon::View_qt::asQWidget(parent));
}

KDDockWidgets::Core::View *CustomWidgetFactory::createSeparator(KDDockWidgets::Core::Separator *controller, KDDockWidgets::Core::View *parent) const {
    return new MySeparator(controller, parent);
}

// QString CustomWidgetFactory::classicIndicatorsPath() const {
// }

MyStack::MyStack(KDDockWidgets::Core::Stack *controller, QWidget *parent)
    : Stack(controller, parent) {
}

void MyStack::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    if (f_theme == Theme::Light) p.fillRect(QWidget::rect(), "#ffffff");
    else p.fillRect(QWidget::rect(), "#292929");
}

MySeparator::MySeparator(KDDockWidgets::Core::Separator *controller, KDDockWidgets::Core::View *parent)
    : Separator(controller, parent) {
}

void MySeparator::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    if (f_theme == Theme::Light) p.fillRect(QWidget::rect(), "#ffffff");
    else p.fillRect(QWidget::rect(), "#292929");

    auto line = QWidget::rect();
    if (line.width() > line.height()) {
        line.setHeight(1);
        line.moveTop((height() - line.height()) / 2);
    } else {
        line.setWidth(1);
        line.moveLeft((width() - line.width()) / 2);
    }
    if (f_theme == Theme::Light) p.fillRect(line, "#d1d1d1");
    else p.fillRect(line, "#666666");
}

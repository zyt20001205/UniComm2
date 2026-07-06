#include "document/assistant/searchWindow.h"

#include <QQmlContext>
#include <QQuickWidget>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include "core/globalManager.h"

SearchWindow::SearchWindow(QObject *parent)
    : QObject(parent),
      m_widget(new QWidget()),
      m_columnLayout(new QVBoxLayout(m_widget)),
      m_quickWidget(new QQuickWidget(m_widget)) {
    m_widget->setWindowTitle("Search");
    m_columnLayout->setContentsMargins(0, 0, 0, 0);
    m_columnLayout->setSpacing(0);
    m_columnLayout->addWidget(m_quickWidget);
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setSource(QUrl("qrc:/qml/document/assistant/searchWindow.qml"));
    m_widget->resize(720, 420);
}

SearchWindow::~SearchWindow() {
    delete m_widget;
}

void SearchWindow::propertySet(const QVariantHash &objects) {
    m_quickWidget->rootContext()->setContextProperty("searchWindow", this);
    m_quickWidget->rootContext()->setContextProperty("global", g_globalManager);
}

void SearchWindow::open() const {
    m_widget->show();
    m_widget->raise();
    m_widget->activateWindow();
}

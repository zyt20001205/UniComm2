#include "mainWindow/statusModule.h"

// StatusModule public
StatusModule::StatusModule(QWidget *parent)
    : QQuickWidget(parent) {
}

StatusModule::~StatusModule() {
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] status module destructed").arg(timestamp);
}

void StatusModule::propertySet(const QVariantMap &objects) {
    setResizeMode(SizeRootObjectToView);
    setSource(QUrl("qrc:/qml/mainWindow/statusModule.qml"));
}

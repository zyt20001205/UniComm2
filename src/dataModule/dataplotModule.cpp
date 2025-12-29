#include "dataModule/dataplotModule.h"

#include <QDateTime>

// DataplotModule public
DataplotModule::DataplotModule()
    : DockWidget("dataplot"){
}

DataplotModule::~DataplotModule() {
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] dataplot module destructed").arg(timestamp);
}

// DataplotModule private
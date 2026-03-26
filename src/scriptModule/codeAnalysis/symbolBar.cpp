#include "scriptModule/codeAnalysis/symbolBar.h"

// public
SymbolBar::SymbolBar(QWidget *parent)
    : QQuickWidget(parent) {
}

void SymbolBar::propertySet(const QVariantMap &objects) {
    setResizeMode(SizeRootObjectToView);
    setSource(QUrl("qrc:/qml/scriptModule/codeAnalysis/symbolBar.qml"));
}

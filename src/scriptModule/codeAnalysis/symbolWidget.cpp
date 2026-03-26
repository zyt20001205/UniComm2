#include "scriptModule/codeAnalysis/symbolWidget.h"

#include <QJsonArray>
#include <QQuickItem>

#include "globals.h"

// public
SymbolWidget::SymbolWidget(QWidget *parent)
    : QQuickWidget(parent) {
    setResizeMode(SizeRootObjectToView);
    setSource(QUrl("qrc:/qml/scriptModule/codeAnalysis/symbolWidget.qml"));
    m_rootItem = rootObject();
}

void SymbolWidget::symbolLoad(const QJsonArray &result, const int line, const int character) const {
    const auto symbolList = symbolParse(result, line, character);
    QMetaObject::invokeMethod(m_rootItem, "symbolLoad", Q_ARG(QVariant, QVariant::fromValue(symbolList)));
}

QVariantList SymbolWidget::symbolParse(const QJsonArray &result, const int line, const int character) {
    QVariantList symbolList{};
    for (const auto &value: result) {
        const auto symbol = value.toObject();
        const auto kind = symbol["kind"].toInt();
        const auto detail = symbol["detail"].toString();
        const auto name = symbol["name"].toString();
        const auto range = symbol["range"].toObject();
        const auto start = range["start"].toObject();
        const auto end = range["end"].toObject();
        if (line < start["line"].toInt() || line > end["line"].toInt()) continue;
        if (line == start["line"].toInt() && character < start["character"].toInt()) continue;
        if (line == end["line"].toInt() && character > end["character"].toInt()) continue;
        QUrl source{};
        switch (kind) {
            case SYMBOL_KIND_PACKAGE: {
                source = "qrc:/icon/symbolPackage.svg";
            }
                break;
            case SYMBOL_KIND_FUNCTION: {
                source = "qrc:/icon/symbolMethod.svg";
            }
            break;
            case SYMBOL_KIND_VARIABLE: {
                source = "qrc:/icon/symbolVariable.svg";
            }
                break;
            case SYMBOL_KIND_CONSTANT: {
                source = "qrc:/icon/symbolConstant.svg";
            }
            break;
            case SYMBOL_KIND_STRING: {
                source = "qrc:/icon/symbolString.svg";
            }
            break;
            case SYMBOL_KIND_NUMBER: {
                source = "qrc:/icon/symbolNumeric.svg";
            }
            break;
            case SYMBOL_KIND_BOOLEAN: {
                source = "qrc:/icon/symbolBoolean.svg";
            }
            break;
            case SYMBOL_KIND_ARRAY: {
                source = "qrc:/icon/symbolArray.svg";
            }
            break;
            case SYMBOL_KIND_OBJECT: {
                source = "qrc:/icon/symbolMisc.svg";
            }
            break;
            default: {
                source = "qrc:/icon/symbolMisc.svg";
                qDebug() << "WIP structure kind:" << kind << name << detail;
            }
            break;
        }
        symbolList.append(QVariantHash{
            {"text", name},
            {"source", source}
        });
        if (symbol.contains("children")) {
            const auto children = symbol["children"].toArray();
            symbolList.append(symbolParse(children, line, character));
        }
        return symbolList;
    }
    return {};
}

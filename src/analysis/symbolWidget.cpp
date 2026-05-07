#include "analysis/symbolWidget.h"

#include <QJsonArray>
#include <QQmlContext>
#include <QQuickItem>

#include "globals.h"

// public
SymbolWidget::SymbolWidget(QWidget *parent)
    : QQuickWidget(parent) {
    setFixedHeight(24);
}

void SymbolWidget::propertySet(const QVariantHash &objects) {
    rootContext()->setContextProperty("symbolWidget", this);
    rootContext()->setContextProperty("global", objects["global"]);
    rootContext()->setContextProperty("mainToolTip", qvariant_cast<QObject *>(objects["mainWindowToolTip"]));

    setResizeMode(SizeRootObjectToView);
    setSource(QUrl("qrc:/qml/analysis/symbolWidget.qml"));
    m_rootItem = rootObject();
}

void SymbolWidget::symbolLoad(const QJsonArray &result, const int line, const int character) {
    const auto symbolList = symbolParse(result, line, character);
    QMetaObject::invokeMethod(m_rootItem, "symbolLoad", Q_ARG(QVariant, QVariant::fromValue(symbolList)));
}

void SymbolWidget::indicatorFill(const QVariantHash &position) {
    emit setFocus(true);
    emit setIndex(
        position["startLine"].toInt(),
        position["startCharacter"].toInt());
    emit fillIndicator(
        ScintillaIndicator::Current,
        position["startLine"].toInt(),
        position["startCharacter"].toInt(),
        position["endLine"].toInt(),
        position["endCharacter"].toInt(),
        1000);
}

QVariantList SymbolWidget::symbolParse(const QJsonArray &result, const int line, const int character) {
    QVariantList symbolList{};
    for (const auto &value: result) {
        const auto symbol = value.toObject();
        const auto kind = symbol["kind"].toInt();
        const auto detail = symbol["detail"].toString().trimmed();
        const auto name = symbol["name"].toString();
        const auto range = symbol["range"].toObject();
        const auto start = range["start"].toObject();
        const auto end = range["end"].toObject();
        if (line < start["line"].toInt() || line > end["line"].toInt()) continue;
        if (line == start["line"].toInt() && character < start["character"].toInt()) continue;
        if (line == end["line"].toInt() && character > end["character"].toInt()) continue;
        const QVariantHash position = {
            {"startLine", start["line"].toInt()},
            {"startCharacter", start["character"].toInt()},
            {"endLine", end["line"].toInt()},
            {"endCharacter", end["character"].toInt()}
        };
        QUrl source{};
        switch (kind) {
            case LspSymbolKind::Package: {
                source = "qrc:/icon/symbolPackage.svg";
            }
            break;
            case LspSymbolKind::Method:
            case LspSymbolKind::Function: {
                source = "qrc:/icon/symbolMethod.svg";
            }
            break;
            case LspSymbolKind::Variable: {
                source = "qrc:/icon/symbolVariable.svg";
            }
            break;
            case LspSymbolKind::Constant: {
                source = "qrc:/icon/symbolConstant.svg";
            }
            break;
            case LspSymbolKind::String: {
                source = "qrc:/icon/symbolString.svg";
            }
            break;
            case LspSymbolKind::Number: {
                source = "qrc:/icon/symbolNumeric.svg";
            }
            break;
            case LspSymbolKind::Boolean: {
                source = "qrc:/icon/symbolBoolean.svg";
            }
            break;
            case LspSymbolKind::Array: {
                source = "qrc:/icon/symbolArray.svg";
            }
            break;
            case LspSymbolKind::Object: {
                source = "qrc:/icon/symbolMisc.svg";
            }
            break;
            default: {
                source = "qrc:/icon/symbolMisc.svg";
                emit appendLog(LogLevel::Warning, "contact author:", QString("unsupported symbol (kind:%1, name:%2, detail:%3)").arg(QString::number(kind), name, detail));
            }
            break;
        }
        symbolList.append(QVariantHash{
            {"text", name},
            {"source", source},
            {"detail", detail},
            {"position", position}
        });
        if (symbol.contains("children")) {
            const auto children = symbol["children"].toArray();
            symbolList.append(symbolParse(children, line, character));
        }
        return symbolList;
    }
    return {};
}

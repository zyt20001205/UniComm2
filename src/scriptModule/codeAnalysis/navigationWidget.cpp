#include "scriptModule/codeAnalysis/navigationWidget.h"

#include <QStandardItemModel>

#include "globals.h"

// NavigationWidget public
NavigationWidget::NavigationWidget(QWidget *parent)
    : QObject(parent),
      m_navigationModel(new QStandardItemModel(this)){
}

void NavigationWidget::propertySet(const QVariantMap &objects) {
    m_tooltip = qvariant_cast<QObject *>(objects["scriptModuleNavigationToolTip"]);
    m_tooltip->setProperty("navigationWidget", QVariant::fromValue(this));
    m_tableView = qvariant_cast<QObject *>(objects["scriptModuleNavigationTableView"]);
    m_tableView->setProperty("model", QVariant::fromValue(m_navigationModel));
    m_label = qvariant_cast<QObject *>(objects["scriptModuleNavigationDetailLabel"]);
}

void NavigationWidget::fontSet(const QString &family, const int pointSize) const {
    if (!m_tooltip) return;
    auto font = m_tooltip->property("font").value<QFont>();
    font.setFamily(family);
    font.setPointSize(pointSize);
    m_tooltip->setProperty("font", font);
}

bool NavigationWidget::isVisible() const {
    if (!m_tooltip) return false;
    return m_tooltip->property("visible").toBool();
}

void NavigationWidget::navigationShow(const QVariantHash &navigationSession, const QJsonArray &navigations) {
    m_navigationSession = navigationSession;
    int row = 0;
    for (const auto &value: navigations) {
        const QJsonObject navigation = value.toObject();
        QString uri = navigation["uri"].toString();
        uri = QUrl::fromPercentEncoding(uri.toUtf8());
        if (QChar &drive = uri[8]; drive.isLetter() && drive.isLower()) { drive = drive.toUpper(); }
        const QUrl scriptUrl(uri);
        const QJsonObject range = navigation["range"].toObject();
        const QJsonObject start = range["start"].toObject();
        const QJsonObject end = range["end"].toObject();
        auto *standardItem = new QStandardItem(scriptUrl.fileName()); // NOLINT
        QUrl iconSource{};
        if (const QString type = navigationSession["type"].toString(); type == "definition") {
            iconSource = "qrc:/icon/definition.svg";
        } else if (type == "implementation") {
            iconSource = "qrc:/icon/implementation.svg";
        } else if (type == "reference") {
            iconSource = "qrc:/icon/reference.svg";
        } else if (type == "typeDefinition") {
            iconSource = "qrc:/icon/typeDefinition.svg";
        }
        standardItem->setData(iconSource, Qt::DecorationRole);
        standardItem->setData(QVariantHash({
            {"scriptUrl", scriptUrl},
            {"startLine", start["line"].toInt()},
            {"startCharacter", start["character"].toInt()},
            {"endLine", end["line"].toInt()},
            {"endCharacter", end["character"].toInt()}
        }), Qt::WhatsThisRole);
        m_navigationModel->appendRow(standardItem);
        row++;
    }
    if (m_navigationModel->rowCount() > 0) {
        const auto position = navigationSession["position"].toPoint();
        m_tooltip->setProperty("position", position);
        m_tableView->setProperty("selectedRow", 0);
        QMetaObject::invokeMethod(m_tooltip, "open");
    } else {
        navigationHide();
    }
}

void NavigationWidget::navigationHide() const {
    QMetaObject::invokeMethod(m_tooltip, "close");
}

void NavigationWidget::navigationPrev() const {
    QMetaObject::invokeMethod(m_tableView, "navigationPrev");
}

void NavigationWidget::navigationNext() const {
    QMetaObject::invokeMethod(m_tableView, "navigationNext");
}

void NavigationWidget::detailReload(const int index) {
    m_detailIndex = index;
    const auto position = m_navigationModel->item(m_detailIndex, 0)->data(Qt::WhatsThisRole).toHash();
    emit getText(position["scriptUrl"].toUrl(), position["startLine"].toInt(), -1, -1, -1);
}

void NavigationWidget::indicatorInsert() {
    const int index = m_tableView->property("selectedRow").toInt();
    const auto position = m_navigationModel->item(index, 0)->data(Qt::WhatsThisRole).toHash();
    emit setCursorPosition(
            position["scriptUrl"].toUrl(),
            position["startLine"].toInt(),
            position["startCharacter"].toInt());
    emit insertIndicator(
        position["scriptUrl"].toUrl(),
        INDICATOR_SELECTION,
        position["startLine"].toInt(),
        position["startCharacter"].toInt(),
        position["endLine"].toInt(),
        position["endCharacter"].toInt(),
        1000);
    navigationHide();
}

void NavigationWidget::navigationResponse(const QString &hint) const {
    const auto position = m_navigationModel->item(m_detailIndex, 0)->data(Qt::WhatsThisRole).toHash();
    const int startCharacter = position["startCharacter"].toInt();
    const int endCharacter = position["endCharacter"].toInt();
    const auto hintText = QString("<span style='white-space: pre;'>%1<span style='color: orange;'>%2</span>%3</span>").arg(
        hint.left(startCharacter).toHtmlEscaped(),
        hint.mid(startCharacter, endCharacter - startCharacter).toHtmlEscaped(),
        hint.mid(endCharacter).toHtmlEscaped());
    m_label->setProperty("text", hintText);
}

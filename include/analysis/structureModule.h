#ifndef UNICOMM_STRUCTUREMODULE_H
#define UNICOMM_STRUCTUREMODULE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QQuickWidget;
class QStandardItem;
class QStandardItemModel;
class QTreeView;

class StructureModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit StructureModule();

    ~StructureModule() override;

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void documentSymbolResponse(const QUrl &documentUrl, const QJsonArray &result);

    void documentFocus(const QUrl &documentUrl, const QVariantHash &session);

    Q_INVOKABLE void markerAdd(const QVariantHash &position);

    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void appendLog(const QString &text, int type);

    void setFocus(const QUrl &documentUrl, bool status);

    void setIndex(const QUrl &documentUrl, int startLine, int startCharacter);

    void addMarker(const QUrl &documentUrl, int type, int line, int time);

private:
    void documentSymbolPublish(const QJsonArray &result, QStandardItem *parentItem);

    QUrl m_currentDocumentUrl{};
    QQuickWidget *m_structureWidget{};
    QObject *m_structureTreeView{};
    QStandardItemModel *m_structureStandardItemModel{};
    QHash<QUrl, QJsonArray> m_documentSymbolHash{};
};

#endif //UNICOMM_STRUCTUREMODULE_H

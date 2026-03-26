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

    void documentSymbolResponse(const QUrl &scriptUrl, const QJsonArray &result);

    void scriptFocus(const QUrl &scriptUrl, const QVariantHash &session);

    Q_INVOKABLE void markerAdd(int row);

signals:
    void addMarker(const QUrl &scriptUrl, int type, int line, int time);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void documentSymbolPublish(const QJsonArray &result, QStandardItem *parentItem) const;

    QUrl m_currentScriptUrl{};
    QQuickWidget *m_structureWidget{};
    QObject *m_structureTreeView{};
    QStandardItemModel *m_structureStandardItemModel{};
    QHash<QUrl, QJsonArray> m_documentSymbolHash{};
};

#endif //UNICOMM_STRUCTUREMODULE_H

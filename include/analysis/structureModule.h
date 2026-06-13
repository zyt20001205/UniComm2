#ifndef UNICOMM_STRUCTUREMODULE_H
#define UNICOMM_STRUCTUREMODULE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QStandardItemModel>

class QQuickWidget;
class QStandardItem;
class QTreeView;

class StructureModel;

class StructureModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit StructureModule();

    ~StructureModule() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void documentSymbolResponse(const QUrl &documentUrl, const QJsonArray &result);

    void documentFocus(const QUrl &documentUrl, const QVariantHash &session);

    Q_INVOKABLE void markerAdd(const QVariantHash &position);

    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void appendLog(int type, const QString &prefix, const QString &message);

    void setFocus(const QUrl &documentUrl, bool status);

    void setIndex(const QUrl &documentUrl, int startLine, int startCharacter);

    void addMarker(const QUrl &documentUrl, int type, int line, int time);

private:
    void documentSymbolPublish(const QJsonArray &result, QStandardItem *parentItem);

    QUrl m_documentUrl{};
    QQuickWidget *m_widget{};
    QObject *m_treeView{};
    StructureModel *m_standardItemModel{};
    QHash<QUrl, QJsonArray> m_documentSymbolHash{};
};

class StructureModel final : public QStandardItemModel {
    Q_OBJECT

public:
    using QStandardItemModel::QStandardItemModel;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
};

#endif //UNICOMM_STRUCTUREMODULE_H

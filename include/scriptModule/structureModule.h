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

    ~StructureModule() override = default;

    void documentSymbolResponse(const QUrl &scriptUrl, const QJsonArray &result);

    void scriptFocus(const QUrl &scriptUrl);

signals:
    void insertMarker(const QUrl &scriptUrl, int type, int line, int time);

private:
    void documentSymbolPublish(const QJsonArray &result, QStandardItem *parentItem) const;

    QUrl m_currentScriptUrl{};
    QQuickWidget *m_structureWidget{};
    QStandardItemModel *m_documentSymbolAbstractModel{};
    QHash<QUrl, QJsonArray> m_documentSymbolHash{};

    enum {
        SYMBOLKIND_FILE = 1,
        SYMBOLKIND_MODULE,
        SYMBOLKIND_NAMESPACE,
        SYMBOLKIND_PACKAGE,
        SYMBOLKIND_CLASS,
        SYMBOLKIND_METHOD,
        SYMBOLKIND_SYMBOLKIND_PROPERTY,
        SYMBOLKIND_FIELD,
        SYMBOLKIND_CONSTRUCTOR,
        SYMBOLKIND_ENUM,
        SYMBOLKIND_INTERFACE,
        SYMBOLKIND_FUNCTION,
        SYMBOLKIND_VARIABLE,
        SYMBOLKIND_CONSTANT,
        SYMBOLKIND_STRING,
        SYMBOLKIND_NUMBER,
        SYMBOLKIND_BOOLEAN,
        SYMBOLKIND_ARRAY,
        SYMBOLKIND_OBJECT,
        SYMBOLKIND_KEY,
        SYMBOLKIND_NULL,
        SYMBOLKIND_ENUMMEMBER,
        SYMBOLKIND_STRUCT,
        SYMBOLKIND_EVENT,
        SYMBOLKIND_OPERATOR,
        SYMBOLKIND_KINDPARAMETER
    };
};

#endif //UNICOMM_STRUCTUREMODULE_H

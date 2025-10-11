#ifndef UNICOMM_STRUCTUREMODULE_H
#define UNICOMM_STRUCTUREMODULE_H

#include <QDockWidget>
#include <QUrl>

class QTreeView;

class StructureModule final : public QDockWidget {
    Q_OBJECT

public:
    explicit StructureModule(QWidget *parent = nullptr);

    ~StructureModule() override = default;

    void documentSymbolReturn(const QUrl &scriptUrl, const QJsonArray &result);

    void scriptSwitch(const QUrl &scriptUrl);

private:
    QUrl m_currentScriptUrl;
    QTreeView *m_documentSymbolTreeView{};
    QHash<QUrl, QJsonArray> m_documentSymbolHash{};
};

#endif //UNICOMM_STRUCTUREMODULE_H

#ifndef UNICOMM_TOOLSMODULE_H
#define UNICOMM_TOOLSMODULE_H

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QSet>

class SqlModule;

class ToolsModule final : public QObject {
    Q_OBJECT

public:
    explicit ToolsModule(SqlModule *sqlModule, QObject *parent = nullptr);

    ~ToolsModule() override = default;

    void initialize();

    [[nodiscard]] QJsonArray toolsGet(const QSet<QString> &names) const;

    [[nodiscard]] QPair<bool, QString> toolCall(int mode, const QString &name, const QString &arguments) const;

    [[nodiscard]] QString toolTextGet(const QString &name, const QString &arguments) const;

    [[nodiscard]] QString toolExecute(const QString &name, const QString &arguments);

signals:
    void updatePlan(const QJsonObject &plan);

private:
    [[nodiscard]] bool permissionGet(int mode, const QString &name) const;

    QHash<QString, int> m_portTypes{};
    QJsonArray m_tools{};
    SqlModule *m_sqlModule{};
    QSet<QString> m_writeGroup{};
    QSet<QString> m_fullAccessGroup{};
};

#endif //UNICOMM_TOOLSMODULE_H

#ifndef UNICOMM_TOOLSMODULE_H
#define UNICOMM_TOOLSMODULE_H

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QPair>
#include <QSet>

class ToolsModule final : public QObject {
    Q_OBJECT

public:
    explicit ToolsModule(QObject *parent = nullptr);

    ~ToolsModule() override = default;

    void initialize();

    [[nodiscard]] QPair<bool, QString> toolCall(const QString &mode, const QString &name, const QString &arguments) const;

    [[nodiscard]] QString toolTextGet(const QString &name, const QString &arguments) const;

    [[nodiscard]] QString toolExecute(const QString &name, const QString &arguments);

signals:
    void registerTools(const QString &name, const QJsonArray &tools);

    void updatePlan(const QJsonObject &plan);

private:
    [[nodiscard]] bool permissionGet(const QString &mode, const QString &name) const;

    QHash<QString, int> m_portTypes{};
    QSet<QString> m_writeGroup{};
    QSet<QString> m_fullAccessGroup{};
};

#endif //UNICOMM_TOOLSMODULE_H

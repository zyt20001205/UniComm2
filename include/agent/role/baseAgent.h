#ifndef UNICOMM_BASEAGENT_H
#define UNICOMM_BASEAGENT_H

#include <QHash>
#include <QJsonArray>
#include <QObject>
#include <QSet>
#include <QString>

class ToolsModule;

class BaseAgent : public QObject {
    Q_OBJECT

public:
    ~BaseAgent() override = default;

    [[nodiscard]] virtual QString idGet() const;

    [[nodiscard]] virtual QString systemGet() const;

    [[nodiscard]] virtual QJsonArray toolsGet(const ToolsModule &toolsModule) const;

    [[nodiscard]] virtual bool planRequired(qsizetype toolCount) const;

protected:
    explicit BaseAgent(QString id, QObject *parent = nullptr);

private:
    QString m_id{};
    QHash<QString, QString> m_systems{};
    QHash<QString, QSet<QString>> m_tools{};
};

#endif //UNICOMM_BASEAGENT_H

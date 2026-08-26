#ifndef UNICOMM_BASEAGENT_H
#define UNICOMM_BASEAGENT_H

#include <QJsonArray>
#include <QObject>
#include <QSet>
#include <QString>

class ToolsModule;

class BaseAgent : public QObject {
    Q_OBJECT

public:
    ~BaseAgent() override = default;

    [[nodiscard]] virtual QString roleGet() const;

    [[nodiscard]] virtual QString systemGet() const = 0;

    [[nodiscard]] virtual QJsonArray toolsGet(const ToolsModule &toolsModule) const;

    [[nodiscard]] virtual bool planRequired(qsizetype toolCount) const;

protected:
    explicit BaseAgent(QString id, QObject *parent = nullptr);

private:
    QString m_role{};
    QHash<QString, QSet<QString>> m_tools{};
};

#endif //UNICOMM_BASEAGENT_H

#ifndef UNICOMM_TERMINALMODULE_H
#define UNICOMM_TERMINALMODULE_H

#include <QJsonObject>

class CmdPage;

class TerminalModule final : public QObject {
    Q_OBJECT

public:
    explicit TerminalModule(QWidget *parent = nullptr);

    ~TerminalModule() override;

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void cmdOpen();

private:
    QJsonObject m_config{};
    QHash<int, CmdPage *> m_cmdHash{};
};

#endif //UNICOMM_TERMINALMODULE_H

#ifndef UNICOMM_TERMINALMODULE_H
#define UNICOMM_TERMINALMODULE_H

#include <QJsonObject>

class CmdPage;
class PowershellPage;

class TerminalModule final : public QObject {
    Q_OBJECT

public:
    explicit TerminalModule(QWidget *parent = nullptr);

    ~TerminalModule() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void cmdOpen();

    Q_INVOKABLE void powershellOpen();

private:
    QJsonObject m_config{};
    QObject *m_global{};
    QHash<int, CmdPage *> m_cmdHash{};
    QHash<int, PowershellPage *> m_powershellHash{};
};

#endif //UNICOMM_TERMINALMODULE_H

#ifndef UNICOMM_TERMINALMODULE_H
#define UNICOMM_TERMINALMODULE_H

#include <QJsonObject>
#include <QStandardItemModel>

class TerminalModel;
class TerminalPage;

class TerminalModule final : public QObject {
    Q_OBJECT

public:
    explicit TerminalModule(QWidget *parent = nullptr);

    ~TerminalModule() override;

    void propertySet(const QVariantHash &objects);

    void terminalConfigSave() const;

    Q_INVOKABLE void terminalOpen(const QString &name, const QString &command);

private:
    QJsonObject m_config{};
    TerminalModel *m_terminalModel{};
    QHash<int, TerminalPage *> m_terminalHash{};
};

class TerminalModel final : public QStandardItemModel {
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCountGet NOTIFY rowCountChanged)

public:
    using QStandardItemModel::QStandardItemModel;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] int rowCountGet () const {
        return rowCount();
    }

signals:
    void rowCountChanged();
};

#endif //UNICOMM_TERMINALMODULE_H

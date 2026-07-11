#ifndef UNICOMM_TERMINALMODULE_H
#define UNICOMM_TERMINALMODULE_H

#include <QJsonObject>
#include <QStandardItemModel>

class QQuickView;

class TerminalModel;
class TerminalPage;

class TerminalModule final : public QObject {
    Q_OBJECT

public:
    explicit TerminalModule(QWidget *parent = nullptr);

    ~TerminalModule() override;

    void propertySet(const QVariantHash &objects);

    void terminalConfigSave() const;

    Q_INVOKABLE void terminalManage() const;

    Q_INVOKABLE void terminalSave();

    Q_INVOKABLE [[nodiscard]] int terminalAdd() const;

    Q_INVOKABLE void terminalDelete(int index) const;

    Q_INVOKABLE void terminalSwap(int src, int dst) const;

    Q_INVOKABLE void terminalOpen(const QString &name, const QVariantHash &session);

private:
    QJsonObject m_config{};
    QQuickView *m_manageWindow{};

    TerminalModel *m_terminalModel{};
    QHash<int, TerminalPage *> m_terminalHash{};
};

class TerminalModel final : public QStandardItemModel {
    Q_OBJECT

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

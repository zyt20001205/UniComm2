#ifndef UNICOMM_STATUSMODULE_H
#define UNICOMM_STATUSMODULE_H

#include <QQuickWidget>
#include <QStandardItemModel>

class BackgroundModel;

class StatusModule final : public QQuickWidget {
    Q_OBJECT

public:
    explicit StatusModule(QWidget *parent = nullptr);

    ~StatusModule() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void backgroundAppend(const QString &name, const std::function<void()> &callback);

    Q_INVOKABLE void backgroundAbort(int taskid);

    Q_INVOKABLE void documentGoto(const QUrl &documentUrl);

    void documentFocus(const QUrl &documentUrl, const QVariantHash &session) const;

    void selectionChange(const QHash<QString, int> &selection) const;

    void threadRefresh(int run, int debug) const;

signals:
    void gotoDocument(const QUrl &documentUrl);

private:
    void backgroundUpdate() const;

    QObject *m_root{};
    QObject *m_positionButton{};
    QObject *m_eolModeButton{};
    QObject *m_codePageButton{};
    QObject *m_threadButton{};
    int m_taskid{};
    QHash<int, std::function<void()>> m_callbacks{};
    BackgroundModel *m_backgroundModel{};
};

class BackgroundModel final : public QStandardItemModel {
    Q_OBJECT
    Q_PROPERTY(QString title READ titleGet NOTIFY titleChanged)
    Q_PROPERTY(int taskid READ taskidGet NOTIFY taskidChanged)

public:
    using QStandardItemModel::QStandardItemModel;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] QString titleGet() const {
        return m_title;
    }

    void titleSet(const QString &title) {
        if (m_title == title) return;
        m_title = title;
        emit titleChanged();
    }

    [[nodiscard]] int taskidGet() const {
        return m_taskid;
    }

    void taskidSet(const int taskid) {
        if (m_taskid == taskid) return;
        m_taskid = taskid;
        emit taskidChanged();
    }

signals:
    void titleChanged();

    void taskidChanged();

private:
    QString m_title{};
    int m_taskid{};
};

#endif //UNICOMM_STATUSMODULE_H
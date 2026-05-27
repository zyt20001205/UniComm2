#ifndef UNICOMM_LLMTOOLS_H
#define UNICOMM_LLMTOOLS_H

#include <QEventLoop>
#include <QJsonArray>

class LLMTools final : public QObject {
    Q_OBJECT

public:
    explicit LLMTools(QObject *parent = nullptr);

    ~LLMTools() override = default;

    [[nodiscard]] QJsonArray toolsGet() {
        return m_tools;
    }

    [[nodiscard]] QString toolsSet(const QString &mode, const QString &name, const QString &arguments);

    void chatCreate(const QString &name, const QJsonObject &object);

    void permissionSet(bool status);

signals:
    void createChat(const QString &role, const QString &text);

    void appendChat(const QString &messageId, const QString &text);

    void setStatus(const QString &status, const QString &text);

private:
    [[nodiscard]] bool permissionGet(const QString &mode, const QString &name, const QJsonObject &object);

    void statusSet(const QString &name, const QJsonObject &object);

    QJsonArray m_tools{};
    QSet<QString> m_writeGroup{};
    QSet<QString> m_godGroup{};
    QEventLoop *m_eventloop{};
    bool m_approved{};
};

#endif //UNICOMM_LLMTOOLS_H

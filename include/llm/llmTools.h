#ifndef UNICOMM_LLMTOOLS_H
#define UNICOMM_LLMTOOLS_H

#include <QJsonArray>

class LLMTools final : public QObject {
     Q_OBJECT

public:
     explicit LLMTools(QObject *parent = nullptr);

     ~LLMTools() override = default;

     [[nodiscard]] QJsonArray toolsGet() {
          return m_tools;
     }

     [[nodiscard]] QString toolsSet(const QString &name, const QString &arguments);

signals:
     void appendChat(const QString &role, const QString &text);

private:
     QJsonArray m_tools{};
};

#endif //UNICOMM_LLMTOOLS_H

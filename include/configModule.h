#ifndef UNICOMM_CONFIG_H
#define UNICOMM_CONFIG_H

#include <QFile>

class ConfigModule final : public QObject {
    Q_OBJECT

public:
    explicit ConfigModule(QWidget *parent = nullptr);

    ~ConfigModule() override = default;

    void configSave(const QString &filePath);

signals:
    void appendLog(const QString &message, const QString &level);

private:
    void configGenerate();

    void configLoad();

    static QJsonObject configValidate(QJsonObject jsonObject);

    QFile m_configFile{};
};

#endif //UNICOMM_CONFIG_H

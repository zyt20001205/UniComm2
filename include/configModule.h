#ifndef UNICOMM_CONFIG_H
#define UNICOMM_CONFIG_H

#include <QObject>
#include <QUrl>

class ConfigModule final : public QObject {
    Q_OBJECT

public:
    explicit ConfigModule(QWidget *parent = nullptr);

    ~ConfigModule() override = default;

    void workspaceOpen();

    void workspaceInit();

    void workspaceConfigSave(QString &filePath);

signals:
    void appendLog(const QString &message, const QString &level);

    void openWorkspace();

private:
    static void mainConfigGenerate();

    static void mainConfigLoad();
};

#endif //UNICOMM_CONFIG_H

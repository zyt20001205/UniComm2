#ifndef UNICOMM_CONFIG_H
#define UNICOMM_CONFIG_H

#include <QObject>
#include <QUrl>

class ConfigModule final : public QObject {
    Q_OBJECT

public:
    explicit ConfigModule(QWidget *parent = nullptr);

    ~ConfigModule() override = default;

    static int mainConfigLoad();

    void workspaceInit();

    void workspaceConfigSave(QString &filePath);

signals:
    void appendLog(const QString &message, const QString &level);

    void openWorkspace();

    void quit();

private:
    static void mainConfigGenerate();
};

#endif //UNICOMM_CONFIG_H

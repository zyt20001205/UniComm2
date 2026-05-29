#ifndef UNICOMM_MCPMODULE_H
#define UNICOMM_MCPMODULE_H

#include <QObject>

class McpModule final : public QObject {
    Q_OBJECT

public:
    explicit McpModule(const QUrl &serverUrl, QObject *parent = nullptr);

    ~McpModule() override = default;
};

#endif //UNICOMM_MCPMODULE_H

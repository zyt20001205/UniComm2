#ifndef UNICOMM_GITMODULE_H
#define UNICOMM_GITMODULE_H

#include "terminal/page/terminalPage.h"

class QTextDocument;
class QQuickWidget;

class GitModule final : public TerminalPage {
    Q_OBJECT

public:
    explicit GitModule(const QString &uniqueName = "Git", const QJsonObject &config = QJsonObject());

    ~GitModule() override = default;
};

#endif //UNICOMM_GITMODULE_H

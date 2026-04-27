#ifndef UNICOMM_CMDPAGE_H
#define UNICOMM_CMDPAGE_H

#include "terminal/page/terminalPage.h"

class QTextDocument;
class QQuickWidget;

class CmdPage final : public TerminalPage {
    Q_OBJECT

public:
    explicit CmdPage(const QString &uniqueName, const QJsonObject &config);

    ~CmdPage() override = default;

protected:
    void closeEvent(QCloseEvent *event) override;

    void processStart() override;
};

#endif //UNICOMM_CMDPAGE_H

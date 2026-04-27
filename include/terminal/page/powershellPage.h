#ifndef UNICOMM_POWERSHELLPAGE_H
#define UNICOMM_POWERSHELLPAGE_H

#include "terminal/page/terminalPage.h"

class QTextDocument;
class QQuickWidget;

class PowershellPage final : public TerminalPage {
    Q_OBJECT

public:
    explicit PowershellPage(const QString &uniqueName, const QJsonObject &config);

    ~PowershellPage() override = default;

protected:
    void closeEvent(QCloseEvent *event) override;

    void processStart() override;
};

#endif //UNICOMM_POWERSHELLPAGE_H

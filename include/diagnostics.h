#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include <QDockWidget>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

class Diagnostics final : public QDockWidget {
    Q_OBJECT

public:
    explicit Diagnostics(QWidget *parent = nullptr);

    ~Diagnostics() override = default;

    void diagnosticsReturn(const QUrl &scriptUrl, const QJsonArray &diagnosticsArray);

signals:
    void openScript(const QUrl &scriptUrl);

    void setCursorPosition(const QUrl &scriptUrl, int startLine, int startCharacter);

    void showIndicator(const QUrl &scriptUrl, int startLine, int startCharacter, int endLine, int endCharacter, int time);

private:
    void diagnosticsClose(int index);

    void diagnosticsPublish(const QUrl &scriptUrl, const QJsonArray &diagnosticsArray);

    void diagnosticsRemove(const QUrl &scriptUrl);

    QTabWidget *m_diagnosticsTabWidget{};
    QHash<int, QColor> m_diagnosticsColor{};
    QHash<QUrl, QTableWidget *> m_diagnosticsTableHash{};

    enum {
        LEVEL_ERROR = 1,
        LEVEL_WARNING,
        LEVEL_INFO,
        LEVEL_HINT
    };
};

#endif //DIAGNOSTICS_H

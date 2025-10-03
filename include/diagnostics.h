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
    void highlightScriptAnnotate(const QUrl &scriptUrl, int startLine, int startCharacter, int endLine, int endCharacter);

private:
    void diagnosticsPublish(const QUrl &scriptUrl, const QJsonArray &diagnosticsArray);

    void diagnosticsRemove(const QUrl &scriptUrl);

    QTabWidget *m_diagnosticsTabWidget = nullptr;
    QHash<int, QColor> m_diagnosticsColor;
    QHash<QUrl, QTableWidget*> m_diagnosticsTableHash{};

    enum {
        SEVERITY_ERROR = 1,
        SEVERITY_WARNING,
        SEVERITY_INFO,
        SEVERITY_HINT
    };
};

#endif //DIAGNOSTICS_H

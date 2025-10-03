#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include <QDockWidget>
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

    void diagnosticsPublish(const QJsonArray &diagnosticsArray) const;

private:
    QTableWidget *m_diagnosticsTableWidget = nullptr;
    QHash<int, QColor> m_diagnosticsColor;
    QHash<QUrl, QJsonArray> m_diagnosticsHash{};

    enum {
        SEVERITY_ERROR = 1,
        SEVERITY_WARNING,
        SEVERITY_INFO,
        SEVERITY_HINT
    };
};

#endif //DIAGNOSTICS_H

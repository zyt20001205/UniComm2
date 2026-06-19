#ifndef UNICOMM_CONFLICTWIDGET_H
#define UNICOMM_CONFLICTWIDGET_H

#include <QJsonArray>

#include "editorWidget.h"

class EditorWidget;

class ConflictWidget final : public EditorWidget {
    Q_OBJECT

public:
    explicit ConflictWidget(const QJsonObject &documentConfig, const QUrl &documentUrl, QWidget *parent = nullptr);

    ~ConflictWidget() override = default;

    void propertySet(const QVariantHash &objects) override;

    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void statResolve(int conflicts);

protected:
    void indicatorInit() const override;

    void markerInit() const override;

private:
    void contentChange();

    QObject *m_toolTip{};
    QTimer *m_contentTimer{};
    QList<int> m_head{};
    QHash<int, QList<int>> m_hunk{};
};

#endif //UNICOMM_CONFLICTWIDGET_H

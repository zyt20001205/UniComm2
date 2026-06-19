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

protected:
    void indicatorInit() const override;

    void marginInit() const override;

    void markerInit() const override;

private:
    void contentChange();

    QObject *m_toolTip{};
    QTimer *m_contentTimer{};
    QHash<int, QList<int>> m_hunk{};
    QList<int> m_head{};
};

#endif //UNICOMM_CONFLICTWIDGET_H

#ifndef UNICOMM_POSITIONWIDGET_H
#define UNICOMM_POSITIONWIDGET_H

#include <QMap>
#include <QObject>

class QTimer;

class PositionWidget final : public QObject {
    Q_OBJECT

public:
    explicit PositionWidget(QObject *parent = nullptr);

    ~PositionWidget() override = default;

    void propertySet(const QVariantHash &objects);

    [[nodiscard]] bool isVisible() const;

    void positionShow(const QVariantMap &positionSession);

    void positionHide() const;

    Q_INVOKABLE void textReplace();

signals:
    void setText(const QUrl &documentUrl, const QString &text, int startLine, int startCharacter, int endLine, int endCharacter);

private:
    QObject *m_tooltip{};
    QVariantMap m_positionSession{};
    QTimer *m_timer{};
};

#endif //UNICOMM_POSITIONWIDGET_H

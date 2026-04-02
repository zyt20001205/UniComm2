#ifndef UNICOMM_SYMBOLWIDGET_H
#define UNICOMM_SYMBOLWIDGET_H

#include <QQuickWidget>

class SymbolWidget final : public QQuickWidget {
    Q_OBJECT

public:
    explicit SymbolWidget(QWidget *parent = nullptr);

    void propertySet(const QVariantMap &objects);

    void symbolLoad(const QJsonArray &result, int line, int character) const;

    Q_INVOKABLE void indicatorFill(const QVariantHash &position);

signals:
    void appendLog(const QString &text, int type);

    void setFocus(bool status);

    void setIndex(int startLine, int startCharacter);

    void fillIndicator(int type, int startLine, int startCharacter, int endLine, int endCharacter, int time);

private:
    static QVariantList symbolParse(const QJsonArray &result, int line, int character);

    QQuickItem *m_rootItem{};
};

#endif //UNICOMM_SYMBOLWIDGET_H

#ifndef UNICOMM_SYMBOLWIDGET_H
#define UNICOMM_SYMBOLWIDGET_H

#include <QQuickWidget>

class SymbolWidget final : public QQuickWidget {
    Q_OBJECT

public:
    explicit SymbolWidget(QWidget *parent = nullptr);

    void symbolLoad(const QJsonArray &result, int line, int character) const;

private:
    static QVariantList symbolParse(const QJsonArray &result, int line, int character);

    QQuickItem *m_rootItem{};
};

#endif //UNICOMM_SYMBOLWIDGET_H
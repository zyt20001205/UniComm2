#ifndef UNICOMM_SCINTILLAWIDGET_H
#define UNICOMM_SCINTILLAWIDGET_H

#include "ScintillaEdit.h"

class QQuickWidget;

class ScintillaWidget final : public ScintillaEdit {
    Q_OBJECT

public:
    explicit ScintillaWidget(const QUrl &scriptUrl, QWidget *parent = nullptr);

    ~ScintillaWidget() override = default;

    void fontSet(const QFont &font);

    void indicatorSet(int indicator, const QJsonObject &config) const;

    void marginSet(int margin, const QJsonObject &config) const;

    void markerSet(int marker, const QJsonObject &config) const;

    void markerAdd(int marker, int line, int time) const;

    void markerDelete(int marker, int line) const;

private:
};

#endif //UNICOMM_SCINTILLAWIDGET_H

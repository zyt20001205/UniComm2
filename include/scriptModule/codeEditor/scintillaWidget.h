#ifndef UNICOMM_SCINTILLAWIDGET_H
#define UNICOMM_SCINTILLAWIDGET_H

#include "ScintillaEdit.h"

class QQuickWidget;

class ScintillaWidget final : public ScintillaEdit {
    Q_OBJECT

public:
    explicit ScintillaWidget(const QUrl &scriptUrl, QWidget *parent = nullptr);

    ~ScintillaWidget() override = default;

    void setFontN(const QFont &font);

    void setIndicator(int indicator, const QJsonObject &config) const;

    void setMargin(int margin, const QJsonObject &config) const;

    void setMarker(int marker, const QJsonObject &config) const;

private:
};

#endif //UNICOMM_SCINTILLAWIDGET_H

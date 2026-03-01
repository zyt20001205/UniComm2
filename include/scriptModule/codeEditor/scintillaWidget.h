#ifndef UNICOMM_SCINTILLAWIDGET_H
#define UNICOMM_SCINTILLAWIDGET_H

#include "ScintillaEdit.h"

class QQuickWidget;

class ScintillaWidget final : public ScintillaEdit {
    Q_OBJECT

public:
    explicit ScintillaWidget(const QUrl &scriptUrl, QWidget *parent = nullptr);

    ~ScintillaWidget() override = default;

    void foldLevelSet(int line, int level) const;

    void fontSet(const QFont &font);

    [[nodiscard]] QHash<QString, int> indexGet(Scintilla::Position position) const;

    void indicatorDefine(int type, const QJsonObject &config) const;

    void indicatorClear(int type, int startLine = -1, int startCharacter = -1, int endLine = -1, int endCharacter = -1) const;

    void indicatorFill(int type, int startLine, int startCharacter, int endLine, int endCharacter, int time = -1) const;

    [[nodiscard]] int lineCountGet() const;

    [[nodiscard]] int lineGet(Scintilla::Position position) const;

    void marginDefine(int type, const QJsonObject &config) const;

    void markerDefine(int type, const QJsonObject &config) const;

    void markerAdd(int type, int line, int time = -1) const;

    void markerDelete(int type, int line) const;

    [[nodiscard]] int markerGet(int line) const;

    [[nodiscard]] Scintilla::Position positionGet(int line, int character = -1) const;

    void savepointSet() const;

    [[nodiscard]] QHash<QString, int> selectionGet() const;

    void styleDefine(int type, const QJsonObject &config) const;

    void textSet(const QString &text) const;

private:
};

#endif //UNICOMM_SCINTILLAWIDGET_H

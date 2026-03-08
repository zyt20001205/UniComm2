#ifndef UNICOMM_SCINTILLAWIDGET_H
#define UNICOMM_SCINTILLAWIDGET_H

#include "ScintillaEdit.h"

class QQuickWidget;

class ScintillaWidget final : public ScintillaEdit {
    Q_OBJECT

public:
    explicit ScintillaWidget(QWidget *parent = nullptr);

    ~ScintillaWidget() override = default;

    [[nodiscard]] bool modifyGet() const;

    void readonlySet(bool status) const;

    void annotationClear() const;

    void annotationSet(int line, const QString &annotation) const;

    void eolAnnotationClear() const;

    void eolAnnotationSet(int line, const QString &annotation) const;

    void foldLevelSet(int line, int level) const;

    void foldContractTop() const;

    void foldContractRecursively() const;

    void foldExpandRecursively() const;

    void fontSet(const QFont &font);

    [[nodiscard]] int heightGet() const;

    [[nodiscard]] QHash<QString, int> indexGet(Scintilla::Position position = -1) const;

    [[nodiscard]] QHash<QString, int> wordIndexGet(Scintilla::Position position, bool onlyWordCharacters = true) const;

    [[nodiscard]] QHash<QString, int> wordIndexGet(int line = -1, int character = -1, bool onlyWordCharacters = true) const;

    void indexSet(int line, int character) const;

    void indicatorDefine(int type, const QJsonObject &config) const;

    void indicatorFill(int type, int startLine, int startCharacter, int endLine, int endCharacter, int time = -1) const;

    void indicatorClear(int type, int startLine = -1, int startCharacter = -1, int endLine = -1, int endCharacter = -1) const;

    [[nodiscard]] int indicatorGet(Scintilla::Position position) const;

    [[nodiscard]] int lineCountGet() const;

    [[nodiscard]] int lineGet(Scintilla::Position position) const;

    void marginDefine(int type, const QJsonObject &config) const;

    void marginTextSet(int line, const QString &text) const;

    void markerDefine(int type, const QJsonObject &config) const;

    void markerAdd(int type, int line, int time = -1) const;

    void markerDelete(int type, int line = -1) const;

    [[nodiscard]] int markerGet(int line) const;

    [[nodiscard]] QHash<QString, int> pointGet(int line, int character) const;

    [[nodiscard]] Scintilla::Position positionGet(int line = -1, int character = -1) const;

    [[nodiscard]] Scintilla::Position positionGet(const QPoint &point) const;

    [[nodiscard]] Scintilla::Position closePositionGet(const QPoint &point) const;

    void positionSet(Scintilla::Position position) const;

    void savepointSet() const;

    [[nodiscard]] QHash<QString, int> selectionGet() const;

    void selectionSet(int startLine, int startCharacter, int endLine, int endCharacter) const;

    void styleDefine(int type, const QJsonObject &config) const;

    [[nodiscard]] int styleGet(Scintilla::Position position) const;

    void styleSet(int type, int startLine = -1, int startCharacter = -1, int length = -1) const;

    void textAppend(const QString &text) const;

    void textClear() const;

    [[nodiscard]] QString textGet(int startLine = -1, int startCharacter = -1, int endLine = -1, int endCharacter = -1) const;

    [[nodiscard]] QString textGetSelected() const;

    void textSet(const QString &text, int startLine = -1, int startCharacter = -1, int endLine = -1, int endCharacter = -1);

    void textSetSelected(const QString &text) const;

private:
};

#endif //UNICOMM_SCINTILLAWIDGET_H

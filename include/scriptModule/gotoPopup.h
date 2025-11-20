#ifndef UNICOMM_GOTOPOPUP_H
#define UNICOMM_GOTOPOPUP_H

#include <QWidget>

class QTableWidget;

class GotoPopup final : public QWidget {
    Q_OBJECT

public:
    explicit GotoPopup(QWidget *parent = nullptr);

    ~GotoPopup() override = default;

    void popupShowDefinition(const QJsonArray &definitions);

    void popupShowReferences(const QJsonArray &references);

    void popupHide();

signals:
    void insertIndicator(const QUrl &scriptUrl, int type, int startLine, int startCharacter, int endLine, int endCharacter, int time);

    void setCursorPosition(const QUrl &scriptUrl, int startLine, int startCharacter);

protected:
    void hideEvent(QHideEvent *event) override;

private:
    void popupGoto(int row);

    QTableWidget *m_tableWidget{};
    QHash<int, QColor> m_gotoColor{};

    enum {
        DIAGNOSTIC,
        REFERENCES
    };
};

#endif //UNICOMM_GOTOPOPUP_H
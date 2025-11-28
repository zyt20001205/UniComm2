#ifndef UNICOMM_GOTOWIDGET_H
#define UNICOMM_GOTOWIDGET_H

#include <QWidget>

class QLabel;
class QListView;
class QStandardItemModel;

class GotoWidget final : public QWidget {
    Q_OBJECT

public:
    explicit GotoWidget(QWidget *parent = nullptr);

    ~GotoWidget() override = default;

    void gotoShowDefinition(const QVariantMap &gotoSession, const QJsonArray &definitions);

    void gotoShowImplementation(const QVariantMap &gotoSession, const QJsonArray &implementations);

    void gotoShowReferences(const QVariantMap &gotoSession, const QJsonArray &references);

    void gotoShowTypeDefinition(const QVariantMap &gotoSession, const QJsonArray &typeDefinitions);

    void gotoHide();

    void gotoPrev();

    void gotoNext();

    void gotoResponse(const QString &hint) const;

signals:
    void setCursorPosition(const QUrl &scriptUrl, int startLine, int startCharacter);

    void getText(const QUrl &scriptUrl, int startLine, int startCharacter, int endLine, int endCharacter);

    void insertIndicator(const QUrl &scriptUrl, int type, int startLine, int startCharacter, int endLine, int endCharacter, int time);

protected:
    void hideEvent(QHideEvent *event) override;

    void leaveEvent(QEvent *event) override;

private:
    void gotoJump(const QModelIndex &index);

    void gotoRequest();

    QVariantMap m_gotoSession{};
    QListView *m_gotoListView{};
    QStandardItemModel *m_gotoModel{};
    QLabel *m_gotoLabel{};

    enum {
        DEFINITION,
        IMPLEMENTATION,
        REFERENCES,
        TYPEDEFINITION
    };
};

#endif //UNICOMM_GOTOWIDGET_H

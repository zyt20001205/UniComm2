#ifndef UNICOMM_NAVIGATIONWIDGET_H
#define UNICOMM_NAVIGATIONWIDGET_H

#include <QWidget>

class QLabel;
class QListView;
class QStandardItemModel;

class NavigationWidget final : public QWidget {
    Q_OBJECT

public:
    explicit NavigationWidget(QWidget *parent = nullptr);

    ~NavigationWidget() override = default;

    void navigationShow(const QVariantMap &navigationSession, const QJsonArray &navigations);

    void navigationHide();

    void navigationPrev();

    void navigationNext();

    void navigationResponse(const QString &hint) const;

signals:
    void setCursorPosition(const QUrl &scriptUrl, int startLine, int startCharacter);

    void getText(const QUrl &scriptUrl, int startLine, int startCharacter, int endLine, int endCharacter);

    void insertIndicator(const QUrl &scriptUrl, int type, int startLine, int startCharacter, int endLine, int endCharacter, int time);

protected:
    void hideEvent(QHideEvent *event) override;

    void leaveEvent(QEvent *event) override;

private:
    void navigationJump(const QModelIndex &index);

    void navigationRequest();

    QVariantMap m_navigationSession{};
    QListView *m_navigationListView{};
    QStandardItemModel *m_navigationModel{};
    QLabel *m_navigationLabel{};

    enum {
        DEFINITION,
        IMPLEMENTATION,
        REFERENCES,
        TYPEDEFINITION
    };
};

#endif //UNICOMM_NAVIGATIONWIDGET_H

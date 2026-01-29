#ifndef UNICOMM_NAVIGATIONWIDGET_H
#define UNICOMM_NAVIGATIONWIDGET_H

#include <QHash>
#include <QObject>

class QLabel;
class QListView;
class QStandardItemModel;

class NavigationWidget final : public QObject {
    Q_OBJECT

public:
    explicit NavigationWidget(QWidget *parent = nullptr);

    ~NavigationWidget() override = default;

    void propertySet(const QVariantMap &objects);

    void fontSet(const QString &family, int pointSize) const;

    bool isVisible() const;

    void navigationShow(const QVariantHash &navigationSession, const QJsonArray &navigations);

    void navigationHide() const;

    void navigationPrev() const;

    void navigationNext() const;

    Q_INVOKABLE void detailReload(int index);

    Q_INVOKABLE void indicatorInsert();

    void navigationResponse(const QString &hint) const;

signals:
    void setCursorPosition(const QUrl &scriptUrl, int startLine, int startCharacter);

    void getText(const QUrl &scriptUrl, int startLine, int startCharacter, int endLine, int endCharacter);

    void insertIndicator(const QUrl &scriptUrl, int type, int startLine, int startCharacter, int endLine, int endCharacter, int time);

private:
    QObject *m_tooltip{};
    QObject *m_tableView{};
    QObject *m_label{};
    QVariantHash m_navigationSession{};
    QStandardItemModel *m_navigationModel{};
    int m_detailIndex = -1;
};

#endif //UNICOMM_NAVIGATIONWIDGET_H

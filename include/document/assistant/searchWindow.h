#ifndef UNICOMM_SEARCHWINDOW_H
#define UNICOMM_SEARCHWINDOW_H

#include <QObject>

class QQuickWidget;
class QVBoxLayout;
class QWidget;

class SearchWindow final : public QObject {
    Q_OBJECT

public:
    explicit SearchWindow(QObject *parent = nullptr);

    ~SearchWindow() override;

    void propertySet(const QVariantHash &objects);

    void open() const;

private:
    QWidget *m_widget{};
    QVBoxLayout *m_columnLayout{};
    QQuickWidget *m_quickWidget{};
};

#endif //UNICOMM_SEARCHWINDOW_H

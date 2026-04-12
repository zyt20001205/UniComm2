#ifndef UNICOMM_BASEPAGE_H
#define UNICOMM_BASEPAGE_H

#include <QJsonObject>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class BasePage : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit BasePage(const QUrl &documentUrl = QUrl());

    ~BasePage() override = default;

    // public: file
    void pathDisambiguation();

    void documentReload();

    void documentClose();

    [[nodiscard]] QUrl documentUrl();

signals:
    void appendLog(const QString &message, int type);

protected:
    void closeEvent(QCloseEvent *event) override;

    void permissionGet();

    QUrl m_documentUrl{};
};

#endif //UNICOMM_BASEPAGE_H

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

    virtual void documentSave() = 0;

    void documentReload();

    [[nodiscard]] QUrl documentUrl();

    virtual void permissionGet();

signals:
    void appendLog(int type, const QString &prefix, const QString &message);

protected:
    void closeEvent(QCloseEvent *event) override;

    virtual bool documentClose();

    QUrl m_documentUrl{};
};

#endif //UNICOMM_BASEPAGE_H

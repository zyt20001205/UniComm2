#ifndef UNICOMM_DOCUMENTPAGE_H
#define UNICOMM_DOCUMENTPAGE_H

#include <QJsonObject>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class DocumentPage : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit DocumentPage(const QUrl &documentUrl = QUrl());

    ~DocumentPage() override = default;

    // public: file
    void pathDisambiguation();

    virtual void documentSave() = 0;

    virtual bool documentClose(bool force = false) {
        return true;
    }

    [[nodiscard]] QUrl documentUrl() {
        return m_documentUrl;
    }

    virtual void permissionGet();

signals:
    void appendLog(int type, const QString &prefix, const QString &message);

    void closeDocument(const QUrl &documentUrl);

protected:
    void closeEvent(QCloseEvent *event) override;

    QUrl m_documentUrl{};
};

#endif //UNICOMM_DOCUMENTPAGE_H

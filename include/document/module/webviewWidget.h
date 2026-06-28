#ifndef UNICOMM_WEBVIEWWIDGET_H
#define UNICOMM_WEBVIEWWIDGET_H

#include <QString>
#include <QUrl>
#include <QWidget>

struct ICoreWebView2;
struct ICoreWebView2Controller;
struct ICoreWebView2Environment;

class WebviewWidget final : public QWidget {
    Q_OBJECT

public:
    explicit WebviewWidget(QWidget *parent = nullptr);

    ~WebviewWidget() override;

    void navigate(const QUrl &url);

    void setHtml(const QString &html);

    void reload() const;

    void stop() const;

    [[nodiscard]] bool isReady() const;

signals:
    void readyChanged(bool ready);

    void loadFinished(bool ok);

    void errorOccurred(const QString &message);

protected:
    void showEvent(QShowEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

private:
    void ensureCreated();

    void resizeWebView() const;

    void closeWebView();

    void registerEvents();

    bool m_oleInitialized{};
    bool m_creating{};
    bool m_ready{};
    QString m_pendingUrl{};
    QString m_pendingHtml{};
    ICoreWebView2Environment *m_environment{};
    ICoreWebView2Controller *m_controller{};
    ICoreWebView2 *m_webView{};
};

#endif //UNICOMM_WEBVIEWWIDGET_H

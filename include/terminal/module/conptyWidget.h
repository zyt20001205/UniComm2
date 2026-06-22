#ifndef UNICOMM_CONPTYWIDGET_H
#define UNICOMM_CONPTYWIDGET_H

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QThread>

class ConptyWidget final : public QObject {
    Q_OBJECT

public:
    explicit ConptyWidget(QObject *parent = nullptr);

    ~ConptyWidget() override;

    [[nodiscard]] bool start(const QString &program, const QString &arguments, const QString &workingDirectory, int rows, int cols);

    void inputWrite(const QByteArray &bytes) const;

    void resize(int rows, int cols) const;

    void stop();

    [[nodiscard]] bool running() const;

signals:
    void outputWrite(const QByteArray &bytes);

    void closed();

private:
    void outputRead();

    static void closeHandle(void *&handle);

    void *m_pseudoConsole{};
    void *m_conptyInputWrite{};
    void *m_conptyOutputRead{};
    void *m_processHandle{};
    void *m_threadHandle{};
    QThread *m_readerThread{};
};

#endif //UNICOMM_CONPTYWIDGET_H

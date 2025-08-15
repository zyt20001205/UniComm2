#ifndef SEND_H
#define SEND_H

#include <QDockWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include "config.h"

class Send final : public QDockWidget {
    Q_OBJECT

public:
    explicit Send(QObject *parent = nullptr);

    ~Send() override = default;

    void commandSend();

private:
    void uiInit();

    QWidget *m_widget = nullptr;
    QHBoxLayout *m_layout = nullptr;
    QTextEdit *m_textEdit = nullptr;

    QVBoxLayout *m_ctrlLayout = nullptr;
    QWidget *m_controlWidget = nullptr;
    QPushButton *m_sendButton = nullptr;

signals:
    void writePort(const QString &command, int index);
};

#endif //SEND_H

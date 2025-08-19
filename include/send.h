#ifndef SEND_H
#define SEND_H

#include <QDockWidget>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include "config.h"

class Send final : public QDockWidget {
    Q_OBJECT

public:
    explicit Send(QObject *parent = nullptr);

    ~Send() override = default;

    void sendConfigSave() const;

    void commandSend();

private:
    QLineEdit *m_textEdit = nullptr;

signals:
    void writePort(const QString &command, int index);
};

#endif //SEND_H

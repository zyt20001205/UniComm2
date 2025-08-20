#include "../include/send.h"

Send::Send(QObject *parent)
    : QDockWidget("send", qobject_cast<QWidget *>(parent)) {
    auto *widget = new QWidget(); // NOLINT
    auto *layout = new QHBoxLayout(widget); // NOLINT
    setWidget(widget);

    m_textEdit = new QLineEdit();
    layout->addWidget(m_textEdit);
    m_textEdit->setText(g_config["sendConfig"].toString());
    auto *sendButton = new QPushButton(); // NOLINT
    layout->addWidget(sendButton);
    sendButton->setFixedSize(24, 24);
    sendButton->setIcon(QIcon(":/icon/send.svg"));
    connect(sendButton, &QPushButton::clicked, this, &Send::commandSend);
}

void Send::commandSend() {
    emit writePort(-1, m_textEdit->text(), "");
}

void Send::sendConfigSave() const {
    g_config["sendConfig"] = m_textEdit->text();
}

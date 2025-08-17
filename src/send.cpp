#include "../include/send.h"

Send::Send(QObject *parent)
    : QDockWidget("send", qobject_cast<QWidget *>(parent)) {
    const auto widget = new QWidget(); // NOLINT
    const auto layout = new QHBoxLayout(widget); // NOLINT
    setWidget(widget);

    m_textEdit = new QTextEdit();
    layout->addWidget(m_textEdit);

    const auto controlWidget = new QWidget(); // NOLINT
    layout->addWidget(controlWidget);
    const auto ctrlLayout = new QVBoxLayout(controlWidget); // NOLINT
    const auto sendButton = new QPushButton("Send"); // NOLINT
    connect(sendButton, &QPushButton::clicked, this, &Send::commandSend);
    ctrlLayout->addWidget(sendButton);
}

void Send::commandSend() {
    emit writePort(m_textEdit->toPlainText(), -1);
}

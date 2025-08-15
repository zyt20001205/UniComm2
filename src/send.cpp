#include "../include/send.h"

Send::Send(QObject *parent)
    : QDockWidget("send", qobject_cast<QWidget *>(parent)) {
    uiInit();
}

void Send::uiInit() {
    m_widget = new QWidget();
    m_layout = new QHBoxLayout(m_widget);
    setWidget(m_widget);

    m_textEdit = new QTextEdit();
    m_layout->addWidget(m_textEdit);

    m_controlWidget = new QWidget();
    m_ctrlLayout = new QVBoxLayout(m_controlWidget);
    m_layout->addWidget(m_controlWidget);
    m_sendButton = new QPushButton("Send");
    connect(m_sendButton, &QPushButton::clicked, this, &Send::commandSend);
    m_ctrlLayout->addWidget(m_sendButton);
}

void Send::commandSend() {
    emit writePort(m_textEdit->toPlainText(), -1);
}

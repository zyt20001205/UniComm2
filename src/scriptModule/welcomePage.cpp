#include "scriptModule/welcomePage.h"

#include <QHBoxLayout>
#include <QPushButton>

// WelcomePage public
WelcomePage::WelcomePage()
    : DockWidget("welcome") {
    auto *widget = new QWidget(); // NOLINT
    setWidget(widget);
    auto *layout = new QHBoxLayout(widget); // NOLINT
    auto *openButton = new QPushButton(tr("Open Workspace")); // NOLINT
    layout->addWidget(openButton);
    connect(openButton, &QPushButton::clicked, this, &WelcomePage::openWorkspace);
}

#include "../include/debug.h"

// Debug public
Debug::Debug(QWidget *parent)
    : QDockWidget("debug", parent),
      m_debugThreadCombobox(new QComboBox()) {
    auto *widget = new QWidget(); // NOLINT
    setWidget(widget);
    auto *layout = new QVBoxLayout(widget); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);

    // debug master control
    {
        auto *debugMasterCtrlWidget = new QWidget(); // NOLINT
        layout->addWidget(debugMasterCtrlWidget);
        auto *debugMasterCtrlLayout = new QVBoxLayout(debugMasterCtrlWidget); // NOLINT
        debugMasterCtrlLayout->setContentsMargins(0, 0, 0, 0);

        auto *debugCtrlWidget = new QWidget(); // NOLINT
        debugMasterCtrlLayout->addWidget(debugCtrlWidget);
        auto *debugCtrlLayout = new QHBoxLayout(debugCtrlWidget); // NOLINT
        debugCtrlLayout->setContentsMargins(0, 0, 0, 0);
        debugCtrlLayout->setAlignment(Qt::AlignLeft);
        auto *debugContinueButton = new QPushButton(); // NOLINT
        debugCtrlLayout->addWidget(debugContinueButton);
        debugContinueButton->setFixedSize(24, 24);
        debugContinueButton->setIcon(QIcon(":/icon/debugContinue.svg"));
        debugContinueButton->setToolTip(tr("resume"));
        // connect(debugContinueButton, &QPushButton::clicked, this, [this] {
        //     g_stateMachine = STATE_RUN;
        //     emit resumeDebug();
        // });
        auto *debugPauseButton = new QPushButton(); // NOLINT
        debugCtrlLayout->addWidget(debugPauseButton);
        debugPauseButton->setFixedSize(24, 24);
        debugPauseButton->setIcon(QIcon(":/icon/pause.svg"));
        debugPauseButton->setToolTip(tr("pause"));
        // connect(debugPauseButton, &QPushButton::clicked, this, [] {
        //     g_stateMachine = STATE_PAUSE;
        // });
        auto *debugStepOverButton = new QPushButton(); // NOLINT
        debugCtrlLayout->addWidget(debugStepOverButton);
        debugStepOverButton->setFixedSize(24, 24);
        debugStepOverButton->setIcon(QIcon(":/icon/debugStepOver.svg"));
        debugStepOverButton->setToolTip(tr("step over"));
        // connect(debugStepOverButton, &QPushButton::clicked, this, [this] {
        //     g_baseDepth = g_depth;
        //     g_stateMachine = STATE_STEPOVER;
        //     emit resumeDebug();
        // });
        auto *debugStepIntoButton = new QPushButton(); // NOLINT
        debugCtrlLayout->addWidget(debugStepIntoButton);
        debugStepIntoButton->setFixedSize(24, 24);
        debugStepIntoButton->setIcon(QIcon(":/icon/debugStepInto.svg"));
        debugStepIntoButton->setToolTip(tr("step into"));
        // connect(debugStepIntoButton, &QPushButton::clicked, this, [this] {
        //     g_stateMachine = STATE_STEPINTO;
        //     emit resumeDebug();
        // });
        auto *debugStepOutButton = new QPushButton(); // NOLINT
        debugCtrlLayout->addWidget(debugStepOutButton);
        debugStepOutButton->setFixedSize(24, 24);
        debugStepOutButton->setIcon(QIcon(":/icon/debugStepOut.svg"));
        debugStepOutButton->setToolTip(tr("step out"));
        // connect(debugStepOutButton, &QPushButton::clicked, this, [this] {
        //     g_baseDepth = g_depth;
        //     g_stateMachine = STATE_STEPOUT;
        //     emit resumeDebug();
        // });
        auto *debugTerminateButton = new QPushButton(); // NOLINT
        debugCtrlLayout->addWidget(debugTerminateButton);
        debugTerminateButton->setFixedSize(24, 24);
        debugTerminateButton->setIcon(QIcon(":/icon/stop.svg"));
        debugTerminateButton->setToolTip(tr("terminate"));
        // connect(debugTerminateButton, &QPushButton::clicked, this, [this] {
        //     g_stateMachine = STATE_TERMINATE;
        //     emit resumeDebug();
        // });

        debugMasterCtrlLayout->addWidget(m_debugThreadCombobox);
    }

    // debug variable treeview
}

void Debug::debugStart() {
    debugThreadGet();
}

// Debug private
void Debug::debugThreadGet() const {
    m_debugThreadCombobox->clear();
    const QHash<QString, QThread *> threadHash = g_threadpool->threadGet();
    QStringList keyList = threadHash.keys();
    foreach (const QString &key, keyList) {
        m_debugThreadCombobox->addItem(key);
    }
}

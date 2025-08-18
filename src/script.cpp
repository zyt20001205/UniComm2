#include "../include/script.h"

static Script *g_script = nullptr;

Script::Script(QWidget *parent)
    : QWidget(parent) {
    // script module init
    {
        const auto layout = new QVBoxLayout(this); // NOLINT
        m_scriptWidget = new QWidget();
        layout->addWidget(m_scriptWidget);
        m_scriptWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        const auto scriptLayout = new QVBoxLayout(m_scriptWidget); // NOLINT
        const auto scriptSplitter = new QSplitter(Qt::Horizontal); // NOLINT
        scriptLayout->addWidget(scriptSplitter);
        m_scriptPlainTextEdit = new ScriptEditor();
        scriptSplitter->addWidget(m_scriptPlainTextEdit);
        m_scriptPlainTextEdit->setPlainText(m_scriptConfig);
        m_scriptPlainTextEdit->document()->setDefaultFont(QFont("Consolas", 11));
        new ScriptHighlighter(m_scriptPlainTextEdit->document());
        m_scriptListWidget = new QListWidget();
        scriptSplitter->addWidget(m_scriptListWidget);
        m_scriptListWidget->setStyleSheet("QListWidget::item { min-height: 40px; }");

        scriptSplitter->setStretchFactor(0, 3);
        scriptSplitter->setStretchFactor(1, 1);

        m_ctrlWidget = new QWidget();
        layout->addWidget(m_ctrlWidget);
        m_ctrlLayout = new QHBoxLayout(m_ctrlWidget);
        auto *runButton = new QPushButton("run"); // NOLINT
        m_ctrlLayout->addWidget(runButton);
        connect(runButton, &QPushButton::clicked, this, &Script::scriptRun);
        auto *saveButton = new QPushButton("save"); // NOLINT
        m_ctrlLayout->addWidget(saveButton);
        connect(saveButton, &QPushButton::clicked, this, &Script::scriptSave);
        auto *helpButton = new QPushButton("help"); // NOLINT
        m_ctrlLayout->addWidget(helpButton);
        connect(helpButton, &QPushButton::clicked, this, [this] {
            m_manualDialog->show();
        });
    }
    manualUiInit();
    g_script = this;
}

void Script::scriptLoad(const QString &scriptPath) {
    QFile file(scriptPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    const QString content = in.readAll();
    file.close();
    m_scriptPlainTextEdit->setPlainText(content);

    const QFileInfo fileInfo(scriptPath);
    QString fileName = fileInfo.fileName();
    emit appendLog(QString("%1 %2").arg(fileName, "loaded"), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, fileName, "loaded");
}

void Script::scriptSave() {
    bool ok;
    QString fileName = QInputDialog::getText(nullptr, "Save Script", "script name:", QLineEdit::Normal, QString(), &ok);
    if (!ok || fileName.isEmpty()) {
        return;
    }
    fileName += ".lua";
    const QString filePath = QDir::current().filePath("script/" + fileName);

    if (QFile::exists(filePath)) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(nullptr, tr("File Exists"), tr("File already exists. Overwrite?"), QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) {
            return;
        }
    }

    QFile file(filePath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << m_scriptPlainTextEdit->toPlainText();
    file.close();

    emit appendLog(QString("%1 %2").arg(fileName, "loaded"), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, fileName, "loaded");
}

void Script::scriptConfigSave() const {
    // m_scriptConfig = m_scriptPlainTextEdit->toPlainText();
    // g_config["scriptConfig"] = m_scriptConfig;
    g_config["scriptConfig"] = m_scriptPlainTextEdit->toPlainText();
}

void Script::manualUiInit() {
    m_manualDialog = new QDialog(this);
    m_manualDialog->setWindowTitle("Manual");
    m_manualDialog->resize(900, 600);
    const auto manualLayout = new QVBoxLayout(m_manualDialog); // NOLINT
    const auto manualSplitter = new QSplitter(Qt::Horizontal); // NOLINT
    manualLayout->addWidget(manualSplitter);

    const auto manualTreeView = new QTreeView(); // NOLINT
    manualSplitter->addWidget(manualTreeView);
    manualTreeView->setHeaderHidden(true);

    const auto manualStandardItemModel = new QStandardItemModel(); // NOLINT
    manualTreeView->setModel(manualStandardItemModel);

    const auto manualPortStandardItem = new QStandardItem("port"); // NOLINT
    manualStandardItemModel->appendRow(manualPortStandardItem);
    const auto manualOpenStandardItem = new QStandardItem("open"); // NOLINT
    manualPortStandardItem->appendRow(manualOpenStandardItem);
    const auto manualCloseStandardItem = new QStandardItem("close"); // NOLINT
    manualPortStandardItem->appendRow(manualCloseStandardItem);
    const auto manualWriteStandardItem = new QStandardItem("write"); // NOLINT
    manualPortStandardItem->appendRow(manualWriteStandardItem);
    const auto manualReadStandardItem = new QStandardItem("read"); // NOLINT
    manualPortStandardItem->appendRow(manualReadStandardItem);

    manualTreeView->expandAll();
    connect(manualTreeView, &QTreeView::clicked, [this](const QModelIndex &index) {
        if (const QString itemText = index.data(Qt::DisplayRole).toString(); itemText == "open") {
            m_manualTextBrowser->setSource(QUrl("open.md"));
        } else if (itemText == "close") {
            m_manualTextBrowser->setSource(QUrl("close.md"));
        } else if (itemText == "write") {
            m_manualTextBrowser->setSource(QUrl("write.md"));
        } else if (itemText == "read") {
            m_manualTextBrowser->setSource(QUrl("read.md"));
        }
    });

    m_manualTextBrowser = new QTextBrowser();
    manualSplitter->addWidget(m_manualTextBrowser);
    m_manualTextBrowser->setOpenExternalLinks(true);
    m_manualTextBrowser->setSearchPaths(QStringList() << ":/doc");

    m_manualTextBrowser->document()->setDefaultFont(QFont("Consolas", 11));

    manualSplitter->setStretchFactor(0, 1);
    manualSplitter->setStretchFactor(1, 3);
}

void Script::scriptRun() {
    const QString script = m_scriptPlainTextEdit->toPlainText();
    if (script.isEmpty()) {
        emit appendLog("script is empty", "warning");
        return;
    }
    // launch lua interpreter thread
    QThread *worker = QThread::create([this, script]() {
        // init lua interpreter
        lua_State *L = luaL_newstate();
        luaL_openlibs(L);
        // register C++ functions
        lua_register(L, "print", Script::luaPrint);
        lua_register(L, "open", Script::luaOpen);
        lua_register(L, "close", Script::luaClose);
        lua_register(L, "write", Script::luaWrite);
        lua_register(L, "read", Script::luaRead);
        lua_register(L, "delay", Script::luaDelay);
        // exec lua script
        const int result = luaL_dostring(L, script.toUtf8().constData());
        if (result != LUA_OK) {
            const QString error = lua_tostring(L, -1);
            emit appendLog(QString("%1 %2").arg("script error:", error), "error");
            lua_pop(L, 1);
        }
        // close interpreter
        lua_close(L);
    });
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    scriptRunning(worker);
    worker->start();
}

void Script::scriptRunning(QThread *worker) {
    auto *scriptListWidgetItem = new QListWidgetItem(); // NOLINT
    m_scriptListWidget->addItem(scriptListWidgetItem);

    connect(worker, &QThread::finished, this, [this,scriptListWidgetItem] {
        const int row = m_scriptListWidget->row(scriptListWidgetItem);
        m_scriptListWidget->takeItem(row);
        delete scriptListWidgetItem;
    });

    auto *scriptInfoWidget = new QWidget(); // NOLINT
    m_scriptListWidget->setItemWidget(scriptListWidgetItem, scriptInfoWidget);
    auto *scriptInfoLayout = new QHBoxLayout(scriptInfoWidget); // NOLINT
    scriptInfoLayout->setContentsMargins(5, 0, 5, 0);
    auto *scriptLabel = new QLabel(QDateTime::currentDateTime().toString("HH:mm:ss") + " editor"); // NOLINT
    scriptInfoLayout->addWidget(scriptLabel);
    auto *abortButton = new QPushButton(); // NOLINT
    scriptInfoLayout->addWidget(abortButton);
    abortButton->setFixedSize(24, 24);
    abortButton->setIcon(QIcon(":/icon/stop.svg"));
    connect(abortButton, &QPushButton::clicked, this, [worker]() {
        worker->terminate();
    });
}

int Script::luaPrint(lua_State *L) {
    const int n = lua_gettop(L);
    QString message;
    for (int i = 1; i <= n; i++) {
        if (const char *str = lua_tostring(L, i)) {
            if (i > 1) message += " ";
            message += QString::fromUtf8(str);
        }
    }
    if (g_script && !message.isEmpty()) {
        emit g_script->appendLog(message, "info");
    }
    return 0;
}

int Script::luaOpen(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 1)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const int param = luaL_optinteger(L, 1, -1);
    // start operation
    emit g_script->openPort(param);
    return 0;
}

int Script::luaClose(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 1)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const int param = luaL_optinteger(L, 1, -1);
    // start operation
    emit g_script->closePort(param);
    return 0;
}

int Script::luaWrite(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 2)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const char *param1 = luaL_checkstring(L, 1);
    const int param2 = luaL_optinteger(L, 2, -1);
    // start operation
    emit g_script->writePort(QString::fromUtf8(param1), param2);
    return 0;
}

int Script::luaRead(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 1)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const int param = luaL_optinteger(L, 1, -1);
    // start operation
    QString data;
    QMetaObject::invokeMethod(g_script->m_port, [&, param]() {
        data = g_script->m_port->portRead(param);
    }, Qt::BlockingQueuedConnection);
    lua_pushstring(L, data.toUtf8().constData());
    return 1;
}

int Script::luaDelay(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 1)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const int param = luaL_checkinteger(L, 1);
    // start operation
    QThread::msleep(param);
    return 0;
}


ScriptEditor::ScriptEditor(QWidget *parent) {
}

ScriptHighlighter::ScriptHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
}

void ScriptHighlighter::highlightBlock(const QString &text) {
    QTextCharFormat format;
    QStringList words;
    // functions
    format.setForeground(QColor("#008683"));
    words = {"close", "delay", "open", "print", "read", "write"};
    foreach(const QString &word, words) {
        QRegularExpression expression("\\b" + word + "\\b");
        QRegularExpressionMatchIterator it = expression.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), format);
        }
    }
    // keywords
    format.setForeground(QColor("#A71D5D"));
    words = {
        "and", "break", "do", "else", "elseif", "end", "false", "for", "function", "goto", "if", "in", "local", "nil", "not", "or", "repeat", "return", "then", "true", "until",
        "while"
    };
    foreach(const QString &word, words) {
        QRegularExpression expression("\\b" + word + "\\b");
        QRegularExpressionMatchIterator it = expression.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), format);
        }
    }
    // string
    format.setForeground(QColor("#183691"));
    QRegularExpression expression(R"xx("([^"\\]|\\.)*")xx");
    QRegularExpressionMatchIterator it = expression.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        setFormat(match.capturedStart(), match.capturedLength(), format);
    }
    // comment
    format.setForeground(QColor("#969896"));
    expression = QRegularExpression(R"(--[^\n]*)");
    it = expression.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        setFormat(match.capturedStart(), match.capturedLength(), format);
    }
}

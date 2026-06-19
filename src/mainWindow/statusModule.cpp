#include "mainWindow/statusModule.h"

#include <QDir>
#include <QQmlContext>
#include <QQuickItem>
#include <Scintilla.h>

#include "globals.h"
#include "core/globalManager.h"

// public
StatusModule::StatusModule(QWidget *parent)
    : QQuickWidget(parent),
      m_backgroundModel(new BackgroundModel(this)) {
}

StatusModule::~StatusModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] status module destructed").arg(timestamp);
}

void StatusModule::propertySet(const QVariantHash &objects) {
    rootContext()->setContextProperty("global", g_globalManager);

    rootContext()->setContextProperty("statusModule", this);
    rootContext()->setContextProperty("workspaceName", g_workspaceUrl.fileName());
    rootContext()->setContextProperty("eolModeMenu", qvariant_cast<QObject *>(objects["statusModuleEolModeMenu"]));
    rootContext()->setContextProperty("backgroundModel", m_backgroundModel);
    setResizeMode(SizeRootObjectToView);
    setSource(QUrl("qrc:/qml/mainWindow/statusModule.qml"));
    m_root = rootObject();
}

void StatusModule::propertyGet(const QVariantMap &objects) {
    m_positionButton = qvariant_cast<QObject *>(objects["positionButton"]);
    m_eolModeButton = qvariant_cast<QObject *>(objects["eolModeButton"]);
    m_codePageButton = qvariant_cast<QObject *>(objects["codePageButton"]);
    m_threadButton = qvariant_cast<QObject *>(objects["threadButton"]);
}

void StatusModule::backgroundAppend(int &taskId, const QString &name, const std::function<void()> &callback) {
    taskId = m_taskId++;
    if (callback) {
        auto *item = new QStandardItem(name); // NOLINT
        item->setData(taskId, Qt::UserRole + 1);
        m_backgroundModel->appendRow(item);
        backgroundUpdate();
        m_callbacks.insert(taskId, callback);
    }
}

void StatusModule::backgroundRemove(const int taskId) const {
    for (int i = 0; i < m_backgroundModel->rowCount(); ++i) {
        if (m_backgroundModel->item(i, 0)->data(Qt::UserRole + 1).toInt() == taskId) {
            m_backgroundModel->removeRow(i);
            backgroundUpdate();
            break;
        }
    }
}

void StatusModule::backgroundRefresh(const int taskId, const QString &message) const {
    for (int i = 0; i < m_backgroundModel->rowCount(); ++i) {
        auto item = m_backgroundModel->item(i, 0);
        if (item->data(Qt::UserRole + 1).toInt() == taskId) {
            item->setText(message);
            break;
        }
    }
}

void StatusModule::backgroundAbort(const int taskId) {
    backgroundRemove(taskId);
    const auto callback = m_callbacks.take(taskId);
    if (callback) callback();
}


void StatusModule::documentGoto(const QUrl &documentUrl) {
    emit gotoDocument(documentUrl);
}

void StatusModule::documentFocus(const QUrl &documentUrl, const QVariantHash &session) const {
    rootContext()->setContextProperty("documentUrl", documentUrl);
    const QString documentPath = documentUrl.toLocalFile();
    const QString workspacePath = g_workspaceUrl.toLocalFile();
    const QDir workspaceDir(workspacePath);
    const QString relativePath = workspaceDir.relativeFilePath(documentPath);
    const QVariantList pathList = QVariant::fromValue(relativePath.split('/')).toList();
    QMetaObject::invokeMethod(m_root, "documentPathLoad", Q_ARG(QVariant, QVariant::fromValue(pathList)));

    switch (session["eolMode"].toInt()) {
        case SC_EOL_CRLF: m_eolModeButton->setProperty("text", tr("CRLF"));
            break;
        case SC_EOL_CR: m_eolModeButton->setProperty("text", tr("CR"));
            break;
        case SC_EOL_LF: m_eolModeButton->setProperty("text", tr("LF"));
            break;
        default: break;
    }

    switch (session["codePage"].toInt()) {
        case 65001: m_codePageButton->setProperty("text", "UTF-8");
            break;
        case 932: m_codePageButton->setProperty("text", "Shift-JIS");
            break;
        case 936: m_codePageButton->setProperty("text", "GBK");
            break;
        case 949: m_codePageButton->setProperty("text", "EUC-KR");
            break;
        case 950: m_codePageButton->setProperty("text", "Big5");
            break;
        case 1361: m_codePageButton->setProperty("text", "JOHAB");
            break;
        default: break;
    }
}

void StatusModule::selectionChange(const QHash<QString, int> &selection) const {
    const auto current = QString("%1:%2").arg(QString::number(selection["line"] + 1), QString::number(selection["character"]));
    const auto charsText = selection["characters"] == 0 ? "" : QString(" [%1 chars]").arg(QString::number(selection["characters"]));
    const auto linesText = selection["lines"] == 0 ? "" : QString(" [%1 lines]").arg(QString::number(selection["lines"]));
    m_positionButton->setProperty("text", QString("%1%2%3").arg(current, charsText, linesText));
}

void StatusModule::threadRefresh(const int run, const int debug) const {
    if (run + debug == 0) {
        m_threadButton->setProperty("text", tr("Idle"));
    } else {
        m_threadButton->setProperty("text", QString(tr("Run: %1 Debug: %2")).arg(QString::number(run), QString::number(debug)));
    }
}

// private
void StatusModule::backgroundUpdate() const {
    const auto taskCount = m_backgroundModel->rowCount();
    if (taskCount == 1) {
        const auto item = m_backgroundModel->item(0, 0);
        m_backgroundModel->titleSet(item->text());
        m_backgroundModel->taskIdSet(item->data(Qt::UserRole + 1).toInt());
    } else {
        m_backgroundModel->titleSet(tr("%1 tasks running.").arg(QString::number(taskCount)));
        m_backgroundModel->taskIdSet(-1);
    }
}

// public
QHash<int, QByteArray> BackgroundModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[Qt::UserRole + 1] = "taskId";
    return roles;
}

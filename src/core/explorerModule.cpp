#include "core/explorerModule.h"

#include <QFileSystemModel>
#include <QFileSystemWatcher>
#include <QProcess>
#include <QQmlContext>
#include <QQuickWidget>

#include "globals.h"

// public
ExplorerModule::ExplorerModule()
    : DockWidget("Explorer"),
      m_widget(new QQuickWidget()),
      m_fileSystemModel(new QFileSystemModel()),
      m_sortFilterProxyModel(new SortFilterProxyModel(&m_documentStatus)),
      m_process(new QProcess()),
      m_gitStatus{
          {'?', GitStatus::Untracked},
          {'!', GitStatus::Ignored},
          {' ', GitStatus::Unmodified},
          {'M', GitStatus::Modified},
          {'T', GitStatus::FileTypeChanged},
          {'A', GitStatus::Added},
          {'D', GitStatus::Deleted},
          {'R', GitStatus::Renamed},
          {'C', GitStatus::Copied},
          {'U', GitStatus::UpdatedButUnmerged},
      } {
    setWidget(m_widget);
    m_widget->installEventFilter(this);
    m_process->setWorkingDirectory(g_workspaceUrl.toLocalFile());
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this] {
        const auto output = m_process->readAllStandardOutput();
        const auto lines = QString(output).split('\n', Qt::SkipEmptyParts);
        m_documentStatus.clear();
        for (const auto &line: lines) {
            const auto indexStatus = line.at(0);
            const auto workingTreeStatus = line.at(1);
            const auto filePath = line.mid(3).trimmed();
            const auto documentPath = QDir(g_workspaceUrl.toLocalFile()).filePath(filePath);
            const auto documentUrl = QUrl::fromLocalFile(documentPath);
            m_documentStatus[documentUrl] = QVariantHash{
                {"indexStatus", m_gitStatus[indexStatus]},
                {"workingTreeStatus", m_gitStatus[workingTreeStatus]}
            };
        }
        m_sortFilterProxyModel->invalidate();
    });
}

ExplorerModule::~ExplorerModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void ExplorerModule::propertySet(const QVariantMap &objects) {
    m_widget->rootContext()->setContextProperty("global", objects["global"]);
    m_widget->rootContext()->setContextProperty("mainToolTip", qvariant_cast<QObject *>(objects["mainWindowToolTip"]));
    m_fileMenu = qvariant_cast<QObject *>(objects["explorerModuleFileMenu"]);
    m_fileMenu->setProperty("gitEnabled", g_gitEnabled);
    m_widget->rootContext()->setContextProperty("fileMenu", m_fileMenu);
    m_folderMenu = qvariant_cast<QObject *>(objects["explorerModuleFolderMenu"]);
    m_folderMenu->setProperty("gitEnabled", g_gitEnabled);
    m_widget->rootContext()->setContextProperty("folderMenu", m_folderMenu);
    m_rootMenu = qvariant_cast<QObject *>(objects["explorerModuleRootMenu"]);
    m_rootMenu->setProperty("gitEnabled", g_gitEnabled);
    m_widget->rootContext()->setContextProperty("rootMenu", m_rootMenu);

    const auto modelRootPath = g_workspaceUrl.toLocalFile();
    m_fileSystemModel->setRootPath(modelRootPath);
    m_sortFilterProxyModel->setSourceModel(m_fileSystemModel);
    const QModelIndex modelRootIndex = m_sortFilterProxyModel->mapFromSource(m_fileSystemModel->index(modelRootPath));
    m_widget->rootContext()->setContextProperty("explorerModule", this);
    m_widget->rootContext()->setContextProperty("modelRootIndex", modelRootIndex);
    m_widget->rootContext()->setContextProperty("modelRootPath", modelRootPath);
    m_widget->rootContext()->setContextProperty("modelRootUrl", g_workspaceUrl);
    m_widget->rootContext()->setContextProperty("sortFilterProxyModel", m_sortFilterProxyModel);
    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/core/explorerModule.qml"));
}

void ExplorerModule::propertyGet(const QVariantMap &objects) {
    m_treeView = qvariant_cast<QObject *>(objects["treeView"]);

    if (g_gitEnabled) gitUpdate();
}

void ExplorerModule::gitInit(const bool status) const {
    m_fileMenu->setProperty("gitEnabled", status);
    m_folderMenu->setProperty("gitEnabled", status);
    m_rootMenu->setProperty("gitEnabled", status);
}

void ExplorerModule::gitUpdate() const {
    m_process->start("git", {"status", "--porcelain", "-uall", "--ignored"});
}

void ExplorerModule::toggleHidden() const {
    auto filters = m_fileSystemModel->filter();
    if (filters.testFlag(QDir::Hidden)) filters &= ~QDir::Hidden;
    else filters |= QDir::Hidden;
    m_fileSystemModel->setFilter(filters);
}

void ExplorerModule::scriptRun(const QUrl &documentUrl) {
    emit startThread(documentUrl, InterpreterMode::Run, -1, -1, -1, -1);
}

void ExplorerModule::scriptDebug(const QUrl &documentUrl) {
    emit startThread(documentUrl, InterpreterMode::Debug, -1, -1, -1, -1);
}

void ExplorerModule::documentOpen(const QUrl &documentUrl) {
    emit openDocument(documentUrl);
}

bool ExplorerModule::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_widget) {
        if (event->type() == QEvent::FocusOut) {
            m_treeView->setProperty("selectedRow", -1);
        }
    }
    return DockWidget::eventFilter(watched, event);
}

SortFilterProxyModel::SortFilterProxyModel(const QHash<QUrl, QVariant> *documentStatus, QObject *parent)
    : QSortFilterProxyModel(parent),
      m_documentStatus(documentStatus) {
}

QHash<int, QByteArray> SortFilterProxyModel::roleNames() const {
    auto roles = QSortFilterProxyModel::roleNames();
    roles[Qt::UserRole + 4] = "source";
    roles[Qt::UserRole + 5] = "isDir";
    roles[Qt::UserRole + 6] = "documentUrl";
    roles[Qt::UserRole + 7] = "git";
    return roles;
}

QVariant SortFilterProxyModel::data(const QModelIndex &index, const int role) const {
    const auto sourceIndex = mapToSource(index);
    const auto *fileModel = qobject_cast<QFileSystemModel *>(sourceModel());
    const auto fileInfo = fileModel->fileInfo(sourceIndex);
    const auto documentUrl = QUrl::fromLocalFile(fileInfo.filePath());
    if (role == Qt::UserRole + 4) {
        QUrl source{};
        const auto suffix = fileInfo.suffix();
        const QStringList imageType = {"bmp", "gif", "ico", "jpeg", "jpg", "png", "svg", "tif", "tiff", "webp"};
        if (imageType.contains(suffix)) {
            source = "qrc:/icon/fileTypeImage.svg";
        } else if (suffix == "csv") {
            source = "qrc:/icon/fileTypeCsv.svg";
        } else if (suffix == "gitignore") {
            source = "qrc:/icon/fileTypeGit.svg";
        } else if (suffix == "json") {
            source = "qrc:/icon/fileTypeJson.svg";
        } else if (suffix == "lua") {
            source = "qrc:/icon/fileTypeLua.svg";
        } else if (fileInfo.isDir()) {
            source = "qrc:/icon/fileTypeFolder.svg";
        } else {
            source = "qrc:/icon/fileTypeDefault.svg";
        }
        return source;
    }
    if (role == Qt::UserRole + 5) {
        return fileInfo.isDir();
    }
    if (role == Qt::UserRole + 6) {
        return documentUrl;
    }
    if (role == Qt::UserRole + 7) {
        if (!g_gitEnabled || !m_documentStatus->contains(documentUrl)) {
            return {};
        }
        const auto gitStatus = m_documentStatus->value(documentUrl).toHash();
        const auto indexStatus = gitStatus["indexStatus"].toInt();
        const auto workingTreeStatus = gitStatus["workingTreeStatus"].toInt();
        return QVariantHash{
            {"indexStatus", indexStatus},
            {"workingTreeStatus", workingTreeStatus}
        };
    }
    return QSortFilterProxyModel::data(index, role);
}

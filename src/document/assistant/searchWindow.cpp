#include "document/assistant/searchWindow.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlContext>
#include <QProcess>
#include <QQuickWidget>
#include <QRegularExpression>
#include <QStringList>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include "core/globalManager.h"
#include "globals.h"

// public
SearchWindow::SearchWindow(QObject *parent)
    : QObject(parent),
      m_widget(new QWidget()),
      m_columnLayout(new QVBoxLayout(m_widget)),
      m_searchWidget(new QQuickWidget(m_widget)),
      m_searchModel(new SearchModel(this)) {
    m_widget->setWindowTitle("Search");
    m_columnLayout->setContentsMargins(0, 0, 0, 0);
    m_columnLayout->setSpacing(0);
    m_columnLayout->addWidget(m_searchWidget);
    m_widget->resize(800, 600);
}

SearchWindow::~SearchWindow() {
    delete m_widget;
}

void SearchWindow::propertySet(const QVariantHash &objects) {
    m_searchWidget->rootContext()->setContextProperty("searchWindow", this);
    m_searchWidget->rootContext()->setContextProperty("global", g_globalManager);
    m_searchWidget->rootContext()->setContextProperty("mainToolTip", qvariant_cast<QObject *>(objects["mainWindowToolTip"]));
    m_searchWidget->rootContext()->setContextProperty("searchModel", m_searchModel);

    m_searchWidget->setFixedHeight(300);
    m_searchWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_searchWidget->setSource(QUrl("qrc:/qml/document/assistant/searchWindow.qml"));
}

void SearchWindow::propertyGet(const QVariantMap &objects) {
    m_searchBar = qvariant_cast<QObject *>(objects["searchBar"]);
    m_searchTextField = qvariant_cast<QObject *>(objects["searchTextField"]);
    m_searchPrevButton = qvariant_cast<QObject *>(objects["searchPrevButton"]);
    m_searchNextButton = qvariant_cast<QObject *>(objects["searchNextButton"]);
    m_searchStatLabel = qvariant_cast<QObject *>(objects["searchStatLabel"]);
    m_replaceBar = qvariant_cast<QObject *>(objects["replaceBar"]);
    m_replaceTextField = qvariant_cast<QObject *>(objects["replaceTextField"]);
    m_replaceTextButton = qvariant_cast<QObject *>(objects["replaceTextButton"]);
    m_replaceAllButton = qvariant_cast<QObject *>(objects["replaceAllButton"]);
}

void SearchWindow::searchRequest() const {
    m_searchModel->clear();
    const auto text = m_searchTextField->property("text").toString();
    if (text.isEmpty()) return;
    // fill pattern
    QString pattern = text;
    QStringList args{};
    args << "--json";
    if (!m_searchFlags.matchCase) args << "--ignore-case";
    if (m_searchFlags.wholeWord) args << "--word-regexp";
    if (m_searchFlags.wordStart) {
        if (!m_searchFlags.regExp) pattern = QRegularExpression::escape(pattern);
        pattern = "\\b" + pattern;
    } else if (!m_searchFlags.regExp) {
        args << "--fixed-strings";
    }
    args << "--" << pattern << g_workspaceUrl.toLocalFile();
    // start search
    QProcess rg{};
    rg.start(QCoreApplication::applicationDirPath() + "/ripgrep/rg.exe", args);
    if (!rg.waitForFinished() || (rg.exitCode() != 0 && rg.exitCode() != 1)) return;
    // parse result
    int matchCount = 0;
    const auto out = rg.readAllStandardOutput();
    for (const auto &line: out.split('\n')) {
        if (line.isEmpty()) continue;
        const auto json = QJsonDocument::fromJson(line).object();
        if (json.value("type").toString() != "match") continue;

        const auto data = json.value("data").toObject();
        const auto path = data.value("path").toObject().value("text").toString();
        const auto relativePath = QDir(g_workspaceUrl.toLocalFile()).relativeFilePath(path);
        const auto _line = data.value("line_number").toInt();
        const auto _text = data.value("lines").toObject().value("text").toString().trimmed();

        auto *pathItem = new QStandardItem(relativePath); // NOLINT
        auto *lineItem = new QStandardItem(QString::number(_line)); // NOLINT
        auto *textItem = new QStandardItem(_text); // NOLINT
        const auto documentUrl = QUrl::fromLocalFile(path).toString();
        pathItem->setData(documentUrl, Qt::UserRole + 1);
        lineItem->setData(documentUrl, Qt::UserRole + 1);
        textItem->setData(documentUrl, Qt::UserRole + 1);

        m_searchModel->appendRow({pathItem, lineItem, textItem});
        ++matchCount;
    }
    m_searchStatLabel->setProperty("matchCount", matchCount);
}

void SearchWindow::open() const {
    m_widget->show();
    m_widget->raise();
    m_widget->activateWindow();
}

void SearchWindow::searchFlagsSet(const bool matchCase, const bool wholeWord, const bool wordStart, const bool regExp) {
    m_searchFlags.matchCase = matchCase;
    m_searchFlags.wholeWord = wholeWord;
    m_searchFlags.wordStart = wordStart;
    m_searchFlags.regExp = regExp;
}

// public
SearchModel::SearchModel(QObject *parent)
    : QStandardItemModel(parent) {
    connect(this, &QAbstractItemModel::rowsInserted, this, &SearchModel::emptyChanged);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &SearchModel::emptyChanged);
    connect(this, &QAbstractItemModel::modelReset, this, &SearchModel::emptyChanged);
}

QHash<int, QByteArray> SearchModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[Qt::UserRole + 1] = "documentUrl";
    return roles;
}
